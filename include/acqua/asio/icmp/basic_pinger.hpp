#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/bind.hpp>
#include <boost/variant.hpp>
#include <boost/function.hpp>
#include <chrono>
#include <mutex>
#include <algorithm>

namespace acqua { namespace asio { namespace icmp {

template <typename Base>
class basic_pinger
    : private Base
{
private:
    using self_type = basic_pinger;

public:
    using clock_type = std::chrono::steady_clock;
    using time_point = typename clock_type::time_point;
    using duration = typename clock_type::duration;
    using endpoint_type = boost::asio::ip::icmp::endpoint;

private:
    struct check_tag {};
    struct search_tag {};
    using buffer_type = std::array<char, 1500>;
    using check_handler_type = boost::function<void(boost::system::error_code const &, endpoint_type)>;
    using search_handler_arg = std::vector<endpoint_type>;
    using search_handler_type = std::tuple<boost::function<void(boost::system::error_code const &, search_handler_arg) >, search_handler_arg>;
    struct entry
    {
        std::uint16_t seq_;
        time_point expire_;
        boost::variant<check_handler_type, search_handler_type> handler_;

        template <typename Handler>
        explicit entry(check_tag, std::uint16_t seq, time_point const & expire, Handler handler)
            : seq_(seq), expire_(expire), handler_(handler) {}

        template <typename Handler>
        explicit entry(search_tag, std::uint16_t seq, time_point const & expire, Handler handler)
            : seq_(seq), expire_(expire), handler_(search_handler_type{handler, search_handler_arg{}}) {}

        friend bool operator<(entry const & lhs, entry const & rhs) noexcept
        {
            return lhs.expire_ < rhs.expire_;
        }
    };

    template <typename Tag, typename Handler>
    void on_resolve(boost::system::error_code const & error, boost::asio::ip::icmp::resolver::iterator it,
                          time_point expire, Handler handler)
    {
        if (error) {
            on_error(Tag{}, error, handler);
            return;
        }

        for(; it != decltype(it)(); ++it) {
            auto ep = it->endpoint();
            if (Base::check(ep)) {
                send_ping<Tag>(ep, expire, handler);
                return;
            }
        }
    }

    template <typename Tag, typename Handler>
    void send_ping(boost::asio::ip::icmp::endpoint const & remote, time_point const & expire, Handler handler)
    {
         if (!socket_.is_open()) {
             on_error(Tag{}, make_error_code(boost::asio::error::bad_descriptor), handler);
             return;
         }

         buffer_type sendbuf;
         auto beg = sendbuf.begin(), end = sendbuf.end();
         auto * echo = Base::generate(beg, end);
         if (!echo) {
             on_error(Tag{}, make_error_code(boost::asio::error::no_buffer_space), handler);
             return;
         }

         echo->id(id_);
         echo->seq(seq_++);
         static char data[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
         for(uint i = 0; i < 16; ++i)
             *end++ = data[i % (sizeof(data)-1)];
         echo->compute_checksum(end);

         std::lock_guard<decltype(mutex_)> lock{mutex_};
         auto it = entries_.emplace_hint(entries_.begin(), Tag{}, echo->seq(), expire, handler);
         if (it != entries_.begin()) {
             mutex_.unlock();
         } else {
             mutex_.unlock();
             timer_.expires_at(expire);
             timer_.async_wait(std::bind(&self_type::on_wait, this, std::placeholders::_1));
         }

         boost::system::error_code ec;
         socket_.send_to(boost::asio::buffer(beg, static_cast<std::size_t>(end - beg)), remote, 0, ec);
         if (ec) {
             std::uint16_t seq = echo->seq();
             mutex_.lock();
             it = std::find_if(entries_.begin(), entries_.end(), [seq](entry const & e) { return e.seq_ == seq; });
             if (it != entries_.end()) {
                 entries_.erase(it);
                 mutex_.unlock();
                 on_error(Tag{}, ec, handler);
                 mutex_.lock();
             }
         }
    }

    void async_receive()
    {
        socket_.async_receive_from(
            boost::asio::buffer(recvbuf_), remote_,
            std::bind(&self_type::on_receive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void on_receive(boost::system::error_code const & error, std::size_t size)
    {
        if (error)
            return;

        auto const * echo = Base::parse(recvbuf_.cbegin(), recvbuf_.cbegin() + size);
        if (!echo || echo->id() != id_) {
            async_receive();
            return;
        }

        auto remote = remote_;
        auto seq = echo->seq();
        async_receive();

        std::lock_guard<decltype(mutex_)> lock{mutex_};
        for(auto it = entries_.begin(); it != entries_.end(); ++it) {
            if (it->seq_ == seq) {
                if (it->handler_.type() == typeid(check_handler_type)) {
                    auto handler = std::move(it->handler_);
                    entries_.erase(it);

                    mutex_.unlock();
                    boost::get<check_handler_type>(handler)(error, remote);
                    mutex_.lock();

                    if (entries_.empty()) {
                        timer_.cancel();
                        return;
                    }
                } else {
                    std::get<1>(boost::get<search_handler_type>(it->handler_)).emplace_back(remote);
                }

                timer_.expires_at(entries_.begin()->expire_);
                timer_.async_wait(std::bind(&self_type::on_wait, this, std::placeholders::_1));
                break;
            }
        }
    }

    struct wait_visitor : boost::static_visitor<>
    {
        void operator()(check_handler_type const & handler) const
        {
            handler(make_error_code(boost::asio::error::timed_out), endpoint_type());
        }

        void operator()(search_handler_type const & args) const
        {
            boost::system::error_code ec;
            if (std::get<1>(args).empty())
                ec = make_error_code(boost::asio::error::timed_out);
            std::get<0>(args)(ec, std::move(std::get<1>(args)));
        }
    };

    void on_wait(boost::system::error_code const & error)
    {
        if (error)
            return;

        std::lock_guard<decltype(mutex_)> lock{mutex_};
        auto it = entries_.begin();
        if (it == entries_.end())
            return;

        auto handler = std::move(it->handler_);
        entries_.erase(it);

        mutex_.unlock();
        boost::apply_visitor(wait_visitor(), handler);
        mutex_.lock();
        if (!entries_.empty()) {
            timer_.expires_at(entries_.begin()->expire_);
            timer_.async_wait(std::bind(&self_type::on_wait, this, std::placeholders::_1));
        }
    }

    template <typename Handler>
    void on_error(check_tag, boost::system::error_code const & error, Handler handler)
    {
        handler(error, endpoint_type{});
    }

    template <typename Handler>
    void on_error(search_tag, boost::system::error_code const & error, Handler handler)
    {
        handler(error, search_handler_arg{});
    }

public:
    explicit basic_pinger(boost::asio::io_service & io_service, int id = -1)
        : socket_(io_service), resolver_(io_service), timer_(io_service)
        , id_(static_cast<std::uint16_t>(id < 0 ? ::getpid() : id))
    {
    }

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec, "start");
    }

    void start(boost::system::error_code & ec)
    {
        Base::open(socket_, ec);
        if (ec) return;
        async_receive();
    }

    void cancel()
    {
        boost::system::error_code ec;
        cancel(ec);
        boost::asio::detail::throw_error(ec, "cancel");
    }

    void cancel(boost::system::error_code & ec)
    {
        socket_.cancel(ec);
    }

    void stop()
    {
        boost::system::error_code ec;
        stop(ec);
        boost::asio::detail::throw_error(ec, "stop");
    }

    void stop(boost::system::error_code & ec)
    {
        socket_.close(ec);
    }

    template <typename Handler>
    void async_check(std::string const & host, time_point const & expire, Handler handler)
    {
        resolver_.async_resolve(
            boost::asio::ip::icmp::resolver::query(host, ""),
            boost::bind(&self_type::on_resolve<check_tag, Handler>, this, _1, _2, expire, handler));
    }

    template <typename Handler>
    void async_check(std::string const & host, duration const & expire, Handler handler)
    {
        resolver_.async_resolve(
            boost::asio::ip::icmp::resolver::query(host, ""),
            boost::bind(&self_type::on_resolve<check_tag, Handler>, this, _1, _2, clock_type::now() + expire, handler));
    }

    template <typename Handler>
    void async_search(std::string const & host, time_point const & expire, Handler handler)
    {
        resolver_.async_resolve(
            boost::asio::ip::icmp::resolver::query(host, ""),
            boost::bind(&self_type::on_resolve<search_tag, Handler>, this, _1, _2, expire, handler));
    }

    template <typename Handler>
    void async_search(std::string const & host, duration const & expire, Handler handler)
    {
        resolver_.async_resolve(
            boost::asio::ip::icmp::resolver::query(host, ""),
            boost::bind(&self_type::on_resolve<search_tag, Handler>, this, _1, _2, clock_type::now() + expire, handler));
    }

private:
    boost::asio::ip::icmp::socket socket_;
    boost::asio::ip::icmp::resolver resolver_;
    boost::asio::steady_timer timer_;
    std::mutex mutex_;

    const std::uint16_t id_;
    std::uint16_t seq_ = 0;
    buffer_type recvbuf_;
    boost::asio::ip::icmp::endpoint remote_;
    boost::container::flat_multiset<entry> entries_;
};

} } }
