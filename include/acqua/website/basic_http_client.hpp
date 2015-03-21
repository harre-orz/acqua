#pragma once

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

public:
    explicit basic_http_client(boost::asio::io_service & io_service, bool volatile & marked_alive)
        : resolver_(io_service), marked_alive_(marked_alive) {}

    ~basic_http_client()
    {
        marked_alive_ = false;
        for(auto const & ptr : kept_sockets_)
            delete ptr;
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
        auto & marked_alive = marked_alive_;
        std::shared_ptr<http_socket> http(
            new http_socket(this, resolver_.get_io_service(), marked_alive),
            [this, &marked_alive](http_socket * http) {
                if (marked_alive && !http->is_keep_alive())
                    collect(http);
                else
                    delete http;
            });
        http->timeout(std::chrono::minutes(1));
        http->async_connect(endpoint);
        return std::static_pointer_cast<socket_type>(http);
    }

    std::shared_ptr<socket_type> make_http_socket(boost::system::error_code const & error, typename resolver_type::iterator it)
    {
        auto & marked_alive = marked_alive_;
        std::shared_ptr<http_socket> http(
            new http_socket(this, resolver_.get_io_service(), marked_alive),
            [this, &marked_alive](http_socket * http) {
                if (marked_alive && !http->is_keep_alive())
                    collect(http);
                else
                    delete http;
            });
        http->timeout(std::chrono::minutes(1));
        http->async_connect(error, it);
        return std::static_pointer_cast<socket_type>(http);
    }

    template <typename SocketPtr>
    void collect(SocketPtr * socket)
    {
        kept_sockets_.emplace(socket);
    }

    template <typename SocketPtr>
    SocketPtr * reuse(SocketPtr * socket)
    {
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
        for(auto it = kept_sockets_.find(socket); it != kept_sockets_.end(); ++it) {
            if (*it == socket) {
                kept_sockets_.erase(it);
                return;
            }

            if (static_cast<SocketPtr const *>(*it)->endpoint_ != socket->endpoint_) {
                return;
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
    std::unordered_set<socket_type *, functor, functor> kept_sockets_;
    bool volatile & marked_alive_;
};

} }
