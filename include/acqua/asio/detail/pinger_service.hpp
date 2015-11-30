#pragma once

#include <functional>
#include <mutex>
#include <atomic>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/variant.hpp>
#include <acqua/asio/socket_category.hpp>

namespace acqua { namespace asio { namespace detail {

template <typename Impl, typename Timer>
class pinger_service
    : public Impl
{
protected:
    using service_type = pinger_service;

public:
    using protocol_type = boost::asio::ip::icmp;
    using endpoint_type = boost::asio::ip::icmp::endpoint;
    using resolver_type = boost::asio::ip::icmp::resolver;
    using socket_type = boost::asio::ip::icmp::socket;
    using timer_type = Timer;
    using time_point = typename timer_type::time_point;

private:
    using category = typename Impl::category;
    using host_handler_type = std::function<void(boost::system::error_code const &, endpoint_type const &)>;
    using multicast_handler_arg1 = std::function<void(boost::system::error_code const &, std::vector<endpoint_type>)>;
    using multicast_handler_type = std::tuple<multicast_handler_arg1, std::vector<endpoint_type> >;
    using buffer_type = std::array<char, 1500>;

    struct entry
    {
        std::uint16_t seq_;
        time_point expire_;
        boost::variant<host_handler_type, multicast_handler_type> handler_;

        explicit entry(int seq, time_point const & expire, host_handler_type handler)
            : seq_(static_cast<std::uint16_t>(seq)), expire_(expire), handler_(handler)
        {
        }

        explicit entry(int seq, time_point const & expire, multicast_handler_arg1 handler)
            : seq_(static_cast<std::uint16_t>(seq)), expire_(expire), handler_(std::make_tuple(handler, std::vector<endpoint_type>()))
        {
        }

        friend bool operator<(entry const & lhs, entry const & rhs) noexcept
        {
            return lhs.expire_ < rhs.expire_;
        }
    };

public:
    explicit pinger_service(boost::asio::io_service & io_service, int id)
        : resolver_(io_service)
        , socket_(io_service)
        , timer_(io_service)
        , id_(static_cast<std::uint16_t>(id)), seq_(0)
    {
    }

    void start(boost::system::error_code & ec)
    {
        if (socket_.is_open()) {
            socket_.cancel(ec);
        } else {
            socket_open(category(), ec);
            if (ec) return;
            socket_bind(category(), ec);
        }
        if (ec) return;

        async_receive();
    }

private:
    void socket_open(internet_v4_tag, boost::system::error_code & ec)
    {
        socket_.open(protocol_type::v4(), ec);
    }

    void socket_open(internet_v6_tag, boost::system::error_code & ec)
    {
        socket_.open(protocol_type::v6(), ec);
    }

    void socket_bind(internet_v4_tag, boost::system::error_code & ec)
    {
        socket_.bind(endpoint_type(boost::asio::ip::address_v4::any(), 0), ec);
    }

    void socket_bind(internet_v6_tag, boost::system::error_code & ec)
    {
        socket_.bind(endpoint_type(boost::asio::ip::address_v6::any(), 0), ec);
    }

public:
    void cancel(boost::system::error_code & ec)
    {
        socket_.cancel(ec);
    }

    void close(boost::system::error_code & ec)
    {
        socket_.close(ec);
    }

    template <typename Handler>
    void ping_host(std::string const & host, time_point const & expire, Handler handler)
    {
        resolver_.async_resolve(
            typename resolver_type::query(host, ""),
            boost::bind(
                &service_type::on_resolve_host<Handler>, this,
                _1, _2, expire, handler));
    }

private:
    template <typename Handler>
    void on_resolve_host(boost::system::error_code const & error, typename resolver_type::iterator it, time_point const & expire, Handler handler)
    {
        if (error) {
            handler(error, endpoint_type());
            return;
        }

        for(; it != typename resolver_type::iterator(); ++it) {
            auto ep = it->endpoint();
            if (check_address(category(), ep)) {
                send_ping(ep, expire, handler);
                return;
            }
        }
        handler(make_error_code(boost::asio::error::no_data), endpoint_type());
    }

    bool check_address(internet_v4_tag, endpoint_type const & ep)
    {
        return ep.address().is_v4();
    }

    bool check_address(internet_v6_tag, endpoint_type const & ep)
    {
        return ep.address().is_v6();
    }

    template <typename Handler>
    void send_ping(endpoint_type const & host, time_point const & expire, Handler handler)
    {
        if (!socket_.is_open()) {
            handler(make_error_code(boost::asio::error::bad_descriptor), endpoint_type());
            return;
        }

        buffer_type buf;
        auto beg = buf.begin(), end = buf.end();
        auto * echo = Impl::generate(beg, end);
        if (!echo) {
            handler(make_error_code(boost::asio::error::no_buffer_space), endpoint_type());
            return;
        }

        echo->id(id_);
        echo->seq(seq_++);
        static char data[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for(uint i = 0; i < 16; ++i)
            *end++ = data[i % (sizeof(data)-1)];
        echo->compute_checksum(end);

        do {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            auto it = entries_.emplace_hint(entries_.begin(), echo->seq(), expire, std::move(handler));
            if (it == entries_.begin()) {
                timer_.expires_at(expire);
                timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
            }
        } while(false);

        boost::system::error_code ec;
        socket_.send_to(boost::asio::buffer(beg, end - beg), host, 0, ec);
        if (ec) {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            auto it = std::find_if(entries_.begin(), entries_.end(),[seq = echo->seq()](auto const & e) { return e.seq_ == seq; });
            if (it != entries_.end()) {
                entries_.erase(it);
                mutex_.unlock();
                handler(ec, host);
            }
        }
    }

    void async_receive()
    {
        socket_.async_receive_from(
            boost::asio::buffer(buffer_), endpoint_,
            std::bind(&service_type::on_receive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void on_receive(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            return;
        }

        auto const * echo = Impl::parse(buffer_.cbegin(), buffer_.cbegin() + size);
        if (!echo || echo->id() != id_) {
            async_receive();
            return;
        }

        auto ep = endpoint_;
        auto seq = echo->seq();
        async_receive();

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        for(auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->seq_ == seq) {
                if (it->handler_.type() == typeid(host_handler_type)) {
                    // ホスト問い合わせのときは、ハンドラを呼ぶ
                    auto handler = std::move(it->handler_);
                    entries_.erase(it);

                    mutex_.unlock();
                    boost::get<host_handler_type>(handler)(error, ep);
                    mutex_.lock();

                    if (entries_.empty()) {
                        timer_.cancel();
                        return;
                    }

                } else /* if (it->handler_.type() == typeid(multicast_args_type)) */ {
                    // マルチキャスト問い合わせのときは、ep を保持する
                    std::get<1>(boost::get<multicast_handler_type>(it->handler_)).emplace_back(ep);
                }

                timer_.expires_at(entries_.begin()->expire_);
                timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
            }
        }
    }

    void on_wait(boost::system::error_code const & error)
    {
        if (!error) {
            std::lock_guard<decltype(mutex_)> lock(mutex_);
            auto it = entries_.begin();
            if (it == entries_.end())
                return;

            auto handler = std::move(it->handler_);
            entries_.erase(it);

            if (handler.type() == typeid(host_handler_type)) {
                // ホスト問い合わせのときは、タイムアウト状態のハンドラを呼ぶ
                mutex_.unlock();
                boost::get<host_handler_type>(handler)(make_error_code(boost::system::errc::timed_out), endpoint_type());
                mutex_.lock();
            } else {
                // マルチキャスト問い合わせのときは、応答元に応じたハンドラを呼ぶ
                mutex_.unlock();
                auto & args = boost::get<multicast_handler_type>(handler);
                boost::system::error_code ec;
                if (!std::get<1>(args).empty())
                    ec = make_error_code(boost::system::errc::timed_out);
                std::get<0>(args)(ec, std::move(std::get<1>(args)));
                mutex_.lock();
            }

            if (!entries_.empty()) {
                timer_.expires_at(entries_.begin()->expire_);
                timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
            }
        }
    }

private:
    resolver_type resolver_;
    socket_type socket_;
    timer_type timer_;
    endpoint_type endpoint_;
    buffer_type buffer_;
    std::uint16_t id_;
    std::atomic<std::uint16_t> seq_;
    boost::container::flat_multiset<entry> entries_;
    std::mutex mutex_;
};

} } }
