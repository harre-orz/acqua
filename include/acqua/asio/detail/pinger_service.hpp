#pragma once

#include <iostream>
#include <functional>
#include <mutex>
#include <atomic>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>
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
    using any_handler_type = boost::function<void(boost::system::error_code const &, endpoint_type)>;
    using all_handler_arg1 = std::vector<endpoint_type>;
    using all_handler_args = boost::function<void(boost::system::error_code const &, all_handler_arg1)>;
    using all_handler_type = std::tuple<all_handler_args, all_handler_arg1>;
    using buffer_type = std::array<char, 1500>;

    struct any_tag {};
    struct all_tag {};

    struct entry
    {
        std::uint16_t seq_;
        time_point expire_;
        boost::variant<any_handler_type, all_handler_type> handler_;

        template <typename Handler>
        entry(uint seq, time_point const & expire, Handler handler, any_tag)
            : seq_(static_cast<std::uint16_t>(seq)), expire_(expire), handler_(handler) {}

        template <typename Handler>
        explicit entry(int seq, time_point const & expire, Handler handler, all_tag)
            : seq_(static_cast<std::uint16_t>(seq)), expire_(expire), handler_(all_handler_type(handler, all_handler_arg1())) {}

        friend bool operator<(entry const & lhs, entry const & rhs) noexcept
        {
            return lhs.expire_ < rhs.expire_;
        }
    };

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

    bool check_endpoint(internet_v4_tag, endpoint_type const & ep)
    {
        return ep.address().is_v4();
    }

    bool check_endpoint(internet_v6_tag, endpoint_type const & ep)
    {
        return ep.address().is_v6();
    }

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

    void cancel(boost::system::error_code & ec)
    {
        socket_.cancel(ec);
    }

    void close(boost::system::error_code & ec)
    {
        socket_.close(ec);
    }

    template <typename Target, typename Handler>
    void ping_any(Target const & target, time_point const & expire, Handler handler)
    {
        ping<any_tag>(target, expire, handler);
    }

    template <typename Target, typename Handler>
    void ping_all(Target const & target, time_point const & expire, Handler handler)
    {
        ping<all_tag>(target, expire, handler);
    }

private:
    template <typename Tag, typename Handler>
    void ping(endpoint_type const & host, time_point const & expire, Handler handler)
    {
        if (check_endpoint(category(), host))
            resolver_.get_io_service().post(boost::bind(&service_type::send_ping<Tag, Handler>, this, host, expire, handler));
        else
            on_error(Tag(), make_error_code(boost::asio::error::address_family_not_supported), handler);
    }

    template <typename Tag, typename Handler>
    void ping(std::string const & host, time_point const & expire, Handler handler)
    {
        resolver_.async_resolve(
            typename resolver_type::query(host, ""),
            boost::bind(&service_type::on_resolve_host<Tag, Handler>, this, _1, _2, expire, handler));
    }

    template <typename Tag, typename Handler>
    void on_resolve_host(boost::system::error_code const & error, typename resolver_type::iterator it, time_point const & expire, Handler handler)
    {
        if (error) {
            on_error(Tag(), error, handler);
            return;
        }

        for(; it != typename resolver_type::iterator(); ++it) {
            auto ep = it->endpoint();
            if (check_endpoint(category(), ep)) {
                send_ping<Tag>(ep, expire, handler);
                return;
            }
        }

        on_error(Tag(), make_error_code(boost::asio::error::no_data), handler);
    }

    template <typename Tag, typename Handler>
    void send_ping(endpoint_type const & ep, time_point const & expire, Handler handler)
    {
        if (!socket_.is_open()) {
            on_error(Tag(), make_error_code(boost::asio::error::bad_descriptor), handler);
            return;
        }

        buffer_type buf;
        auto beg = buf.begin(), end = buf.end();
        auto * echo = Impl::generate(beg, end);
        if (!echo) {
            on_error(Tag(), make_error_code(boost::asio::error::no_buffer_space), handler);
            return;
        }

        echo->id(id_);
        echo->seq(seq_++);
        static char data[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for(uint i = 0; i < 16; ++i)
            *end++ = data[i % (sizeof(data)-1)];
        echo->compute_checksum(end);

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = entries_.emplace_hint(entries_.begin(), echo->seq(), expire, handler, Tag());
        if (it != entries_.begin()) {
            mutex_.unlock();
        } else {
            mutex_.unlock();
            timer_.expires_at(expire);
            timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
        }

        boost::system::error_code ec;
        socket_.send_to(boost::asio::buffer(beg, static_cast<std::size_t>(end - beg)), ep, 0, ec);
        if (ec) {
            mutex_.lock();
            it = std::find_if(entries_.begin(), entries_.end(),[seq = echo->seq()](auto const & e) { return e.seq_ == seq; });
            if (it != entries_.end()) {
                entries_.erase(it);
                mutex_.unlock();
                on_error(Tag(), ec, handler);
                mutex_.lock();
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
                if (it->handler_.type() == typeid(any_handler_type)) {
                    // エニー問い合わせのときは、即座にハンドラを呼ぶ
                    auto handler = std::move(it->handler_);
                    entries_.erase(it);

                    mutex_.unlock();
                    boost::get<any_handler_type>(handler)(error, ep);
                    mutex_.lock();

                    if (entries_.empty()) {
                        timer_.cancel();
                        return;
                    }

                } else // if (it->handler_.type() == typeid(all_args_type))
                    // マルチ問い合わせのときは、配列に格納しておく
                    std::get<1>(boost::get<all_handler_type>(it->handler_)).emplace_back(ep);

                timer_.expires_at(entries_.begin()->expire_);
                timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
                break;
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

            if (handler.type() == typeid(any_handler_type)) {
                // エニー問い合わせのときは、タイムアウト状態のハンドラを呼ぶ
                mutex_.unlock();
                boost::get<any_handler_type>(handler)(make_error_code(boost::asio::error::timed_out), endpoint_type());
            } else {
                // マルチ問い合わせのときは、ハンドラを呼ぶ
                mutex_.unlock();
                auto & args = boost::get<all_handler_type>(handler);
                boost::system::error_code ec;
                if (std::get<1>(args).empty())
                    ec = make_error_code(boost::asio::error::timed_out);
                std::get<0>(args)(ec, std::move(std::get<1>(args)));
            }

            mutex_.lock();
            if (!entries_.empty()) {
                timer_.expires_at(entries_.begin()->expire_);
                timer_.async_wait(std::bind(&service_type::on_wait, this, std::placeholders::_1));
            }
        }
    }

    template <typename Handler>
    void on_error(any_tag, boost::system::error_code const & error, Handler handler)
    {
        handler(error, endpoint_type());
    }

    template <typename Handler>
    void on_error(all_tag, boost::system::error_code const & error, Handler handler)
    {
        handler(error, all_handler_arg1());
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
