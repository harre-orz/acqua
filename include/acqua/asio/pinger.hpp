/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <atomic>
#include <mutex>

#include <boost/array.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/asio/ip/icmp.hpp>
#include <boost/asio/buffer.hpp>
#include <acqua/mref.hpp>
#include <acqua/asio/timer_traits.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace asio {

template <typename Service, typename AsioTimer, std::size_t N = 1500>
class basic_pinger
{
    using buffer_type = boost::array<char, N>;
    using clock_type = acqua::asio::timer_traits<AsioTimer>;

public:
    using service_type = Service;
    using socket_type = boost::asio::ip::icmp::socket;
    using resolver_type = boost::asio::ip::icmp::resolver;
    using endpoint_type = boost::asio::ip::icmp::endpoint;
    using timer_type = typename clock_type::timer_type;
    using duration_type = typename clock_type::duration_type;
    using time_point_type = typename clock_type::time_point_type;

private:
    struct implementation
    {
        time_point_type expiretime;
        endpoint_type endpoint;
        std::uint16_t seq;
        std::function<void(boost::system::error_code const &, endpoint_type const &)> handler;

        template <typename Handler>
        explicit implementation(time_point_type const & expiretime, endpoint_type const & endpoint, std::uint16_t seq, Handler handler)
            : expiretime(expiretime), endpoint(endpoint), seq(seq), handler(handler) {}

        friend bool operator<(implementation const & lhs, implementation const & rhs)
        {
            return lhs.expiretime < rhs.expiretime;
        }
    };

public:
    explicit basic_pinger(boost::asio::io_service & io_service, std::uint16_t id)
        : socket_(io_service, service_type::protocol())
        , resolver_(io_service)
        , timer_(io_service)
        , id_(id)
        , seq_(0)
        , running_(false)
    {
    }

    void start()
    {
        if (running_.exchange(true) == false) {
            async_receive(std::unique_ptr<buffer_type>(new buffer_type()));
        }
    }

    void stop()
    {
        running_ = false;
        socket_.cancel();
    }

    template <typename Handler>
    void ping(std::string const & host, time_point_type const & expiretime, Handler handler, std::size_t length = 48)
    {
        if (running_) {
            boost::system::error_code ec;
            for(auto it = resolver_.resolve(resolver_type::query(host, ""), ec); it != resolver_type::iterator(); ++it) {
                auto endpoint = it->endpoint();
                if (service_type::is_valid_address(endpoint)) {
                    do_ping(std::min((length), N), endpoint, expiretime, handler);
                    return;
                }
            }
        }

        handler(boost::system::error_code(EAFNOSUPPORT, boost::system::generic_category()), endpoint_type());
    }

    template <typename Handler>
    void ping(std::string const & host, duration_type const & expiretime, Handler handler, std::size_t length = 48)
    {
        if (running_) {
            boost::system::error_code ec;
            for(auto it = resolver_.resolve(resolver_type::query(host, ""), ec); it != resolver_type::iterator(); ++it) {
                auto endpoint = it->endpoint();
                if (service_type::is_valid_address(endpoint)) {
                    do_ping(std::min((length), N), endpoint, clock_type::now() + expiretime, handler);
                    return;
                }
            }
        }

        handler(boost::system::error_code(EAFNOSUPPORT, boost::system::generic_category()), endpoint_type());
    }

private:
    template <typename Handler>
    void do_ping(std::size_t length, endpoint_type const & endpoint, time_point_type const & expiretime, Handler handler)
    {
        buffer_type buffer;
        auto * end = buffer.end();
        auto * echo = service_type::make_icmp_echo(buffer.data(), end);
        echo->id(id_);
        echo->seq(seq_++);
        char data[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        for(std::size_t n = 0; n < length; n++)
            *end++ = data[n % (sizeof(data) - 1)];
        echo->compute_checksum(end);

        boost::system::error_code ec;
        socket_.send_to(boost::asio::buffer(buffer.data(), end - buffer.data()), endpoint, 0, ec);
        if (ec) {
            handler(ec, endpoint);
            return;
        }

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = set_.emplace_hint(set_.begin(), expiretime, endpoint, echo->seq(), handler);
        if (it == set_.begin()) {
            timer_.expires_at(it->expiretime);
            timer_.async_wait(std::bind(&basic_pinger::on_wait, this, std::placeholders::_1));
        }
    }

    void async_receive(std::unique_ptr<buffer_type> && buffer)
    {
        buffer_type & buffer_ = *buffer;
        socket_.async_receive_from(
            boost::asio::buffer(buffer_), endpoint_,
            std::bind(
                &basic_pinger::on_receive,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                acqua::mref(std::move(buffer))
            )
        );
    }

    void on_receive(boost::system::error_code const & error, std::size_t size, std::unique_ptr<buffer_type> buffer)
    {
        if (!error) {
            auto const * echo = service_type::parse_icmp_echo(buffer->begin(), buffer->begin() + size);
            if (echo && echo->id() == id_) {
                auto seq = echo->seq();
                auto endpoint = endpoint_;
                async_receive(std::move(buffer));

                std::lock_guard<decltype(mutex_)> lock(mutex_);
                for(auto it = set_.begin(); it != set_.end(); ++it) {
                    if (it->seq == seq) {
                        auto handler = std::move(it->handler);
                        set_.erase(it);
                        mutex_.unlock();

                        handler(error, endpoint);

                        mutex_.lock();
                        if (set_.empty()) {
                            timer_.cancel();
                        } else {
                            timer_.expires_at(set_.begin()->expiretime);
                            timer_.async_wait(std::bind(&basic_pinger::on_wait, this, std::placeholders::_1));
                        }
                        break;
                    }
                }
            } else {
                async_receive(std::move(buffer));
            }
        } else if (running_.exchange(false) == true) {
            acqua::exception::throw_error(error, "pinger");
        }
    }

    void on_wait(boost::system::error_code const & error)
    {
        if (!error) {
            std::lock_guard<decltype(mutex_)> lock(mutex_);

            auto it = set_.begin();
            auto handler = std::move(it->handler);
            auto endpoint = std::move(it->endpoint);
            set_.erase(it);
            mutex_.unlock();

            handler(boost::system::error_code(ETIMEDOUT, boost::system::generic_category()), endpoint);

            mutex_.lock();
            if (!set_.empty()) {
                timer_.expires_at(set_.begin()->expiretime);
                timer_.async_wait(std::bind(&basic_pinger::on_wait, this, std::placeholders::_1));
            }
        }
    }

private:
    socket_type socket_;
    resolver_type resolver_;
    timer_type timer_;
    endpoint_type endpoint_;
    std::uint16_t id_;
    std::uint16_t seq_;
    std::atomic<bool> running_;
    std::mutex mutex_;
    boost::container::flat_multiset<implementation> set_;
};

} }


#include <boost/asio/steady_timer.hpp>
#include <acqua/asio/detail/ping4_service.hpp>
#include <acqua/asio/detail/ping6_service.hpp>

namespace acqua { namespace asio {

typedef basic_pinger<detail::ping4_service, boost::asio::steady_timer> pinger_v4;
typedef basic_pinger<detail::ping6_service, boost::asio::steady_timer> pinger_v6;

} }
