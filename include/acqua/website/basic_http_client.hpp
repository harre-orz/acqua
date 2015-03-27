/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <acqua/website/detail/client_socket_base.hpp>

namespace acqua { namespace website {

template <typename Result, typename Timer>
class basic_http_client
    : private boost::noncopyable
{
    using self_type = basic_http_client<Result, Timer>;
    using http_socket = detail::basic_client_socket<self_type, Result, boost::asio::ip::tcp::socket, Timer>;
    friend http_socket;

public:
    using resolver_type = boost::asio::ip::tcp::resolver;
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using socket_type = detail::client_socket_base<Result>;
    using result_ptr = boost::shared_ptr<Result>;
    using result = typename Result::result;

public:
    explicit basic_http_client(boost::asio::io_service & io_service, std::size_t max_count = 100)
        : resolver_(io_service), max_count_(max_count), count_(0)
    {
        if (max_count <= 0)
            throw boost::system::system_error(EINVAL, boost::system::generic_category());
    }

    ~basic_http_client()
    {
        for(auto const & ptr : kept_sockets_)
            delete ptr;
    }

    std::size_t max_count() const noexcept
    {
        return max_count_;
    }

    std::size_t use_count() const noexcept
    {
        return count_;
    }

    std::size_t keep_count() const noexcept
    {
        return kept_sockets_.size();
    }

    std::shared_ptr<socket_type> http_connect(endpoint_type const & endpoint)
    {
        return make_http_socket(endpoint);
    }

    std::shared_ptr<socket_type> http_connect(boost::asio::ip::address_v4 const & addr, std::uint16_t port = 80)
    {
        return make_http_socket(endpoint_type(addr, port));
    }

    std::shared_ptr<socket_type> http_connect(boost::asio::ip::address_v6 const & addr, std::uint16_t port = 80)
    {
        return make_http_socket(endpoint_type(addr, port));
    }

    std::shared_ptr<socket_type> http_connect(std::string const & host, std::string const & serv)
    {
        boost::system::error_code ec;
        return make_http_socket(ec, resolver_.resolve(typename resolver_type::query(host, serv), ec));
    }

    std::shared_ptr<socket_type> http_connect(std::string const & host)
    {
        boost::system::error_code ec;
        return make_http_socket(ec, resolver_.resolve(typename resolver_type::query(host, "80"), ec));
    }

private:
    std::shared_ptr<socket_type> make_http_socket(endpoint_type const & endpoint)
    {
        lock_wait();

        std::shared_ptr<http_socket> http(
            new http_socket(this, resolver_.get_io_service()),
            [this](http_socket * http) {
                if (http->is_keep_alive())
                    collect(http);
                else
                    delete http;
                --count_;
                cond_.notify_one();
            });
        http->timeout(std::chrono::minutes(1));
        http->async_connect(endpoint);
        return std::static_pointer_cast<socket_type>(http);
    }

    std::shared_ptr<socket_type> make_http_socket(boost::system::error_code const & error, typename resolver_type::iterator it)
    {
        lock_wait();

        std::shared_ptr<http_socket> http(
            new http_socket(this, resolver_.get_io_service()),
            [this](http_socket * http) {
                if (http->is_keep_alive())
                    collect(http);
                else
                    delete http;

                --count_;
                cond_.notify_one();
            });
        http->timeout(std::chrono::minutes(1));
        http->async_connect(error, it);
        return std::static_pointer_cast<socket_type>(http);
    }

    void lock_wait()
    {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        cond_.wait(lock, [this] { return (count_ + kept_sockets_.size()) < max_count_ ; });
        ++count_;
    }

    template <typename SocketPtr>
    void collect(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        kept_sockets_.emplace(socket);
        socket->async_keep_alive();
    }

    template <typename SocketPtr>
    SocketPtr * reuse(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = kept_sockets_.find(socket);
        if (it != kept_sockets_.end()) {
            socket = static_cast<SocketPtr *>(*it);
            kept_sockets_.erase(it);
        } else {
            socket = nullptr;
        }

        return socket;
    }

    template <typename SocketPtr>
    void erase(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        for(auto it = kept_sockets_.find(socket); it != kept_sockets_.end(); ++it) {
            if (*it == socket) {
                kept_sockets_.erase(it);
                cond_.notify_one();
                return;
            }

            if (static_cast<SocketPtr const *>(*it)->endpoint() != socket->endpoint()) {
                break;
            }
        }
    }

private:
    struct functor
    {
        template <typename T>
        bool operator()(T const & lhs, T const & rhs) const noexcept
        {
            return lhs->endpoint() == rhs->endpoint();
        }

        template <typename T>
        std::size_t operator()(T const & rhs) const
        {
            auto const & ep = rhs->endpoint();
            return std::accumulate(
                reinterpret_cast<std::size_t const *>(ep.data()),
                reinterpret_cast<std::size_t const *>(ep.data()) + (ep.size() / sizeof(std::size_t)),
                (std::size_t)0);
        }
    };

    resolver_type resolver_;
    std::size_t max_count_;
    std::atomic<std::size_t> count_;
    std::unordered_multiset<socket_type *, functor, functor> kept_sockets_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

} }
