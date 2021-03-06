/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <chrono>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <acqua/webclient/client_result.hpp>
#include <acqua/webclient/detail/client_socket.hpp>


namespace acqua { namespace webclient {

template <typename Result, typename Timer>
class basic_http_client
    : private boost::noncopyable
{
    using self_type = basic_http_client<Result, Timer>;
    using http_socket = detail::client_socket<self_type, Result, boost::asio::ip::tcp::socket, Timer>;
    using https_socket = detail::client_socket<self_type, Result, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>, Timer>;
    friend http_socket;
    friend https_socket;

public:
    using resolver_type = boost::asio::ip::tcp::resolver;
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using socket_type = detail::client_socket_base<Result>;
    using result_ptr = boost::shared_ptr<Result>;
    using result = typename Result::result;

public:
    explicit basic_http_client(boost::asio::io_service & io_service, std::size_t max_count = 100)
        : resolver_(io_service), context_(nullptr), max_count_(max_count), count_(0)
    {
        if (max_count <= 0)
            throw boost::system::system_error(EINVAL, boost::system::generic_category());
    }

    explicit basic_http_client(boost::asio::io_service & io_service, boost::asio::ssl::context & ctx, std::size_t max_count = 100)
        : resolver_(io_service), context_(&ctx), max_count_(max_count), count_(0)
    {
        if (max_count <= 0)
            throw boost::system::system_error(EINVAL, boost::system::generic_category());
    }

    ~basic_http_client()
    {
        for(auto const & ptr : keep_sockets_)
            delete ptr;
    }

    bool & enabled_keep_alive() { return enabled_keep_alive_; }
    bool const & enabled_keep_alive() const { return enabled_keep_alive_; }

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
        return keep_sockets_.size();
    }

    template <typename Req, typename Uri>
    std::shared_ptr<socket_type> connect(Req const & req, Uri const & uri)
    {
        return do_connect(req, uri, 3, true);
    }

private:
    template <typename Req, typename Uri>
    std::shared_ptr<socket_type> do_connect(Req const & req, Uri const & uri, int retry, bool use_lock)
    {
        boost::system::error_code ec;
        std::shared_ptr<socket_type> socket;
        std::string host;
        std::string serv;
        int scheme;

        namespace qi = boost::spirit::qi;
        qi::symbols<char, int> sym;
        sym.add("http://", 1)("https://", 2);
        auto it = uri.begin();
        if (!qi::parse(it, uri.end(), sym >> *(qi::char_ - ':' - '/') >> -(':' >> qi::char_('0','9')), scheme, host, serv))
            throw std::runtime_error("invalid argument");

        lock_wait(use_lock);

        switch(scheme) {
            case 1:
                if (serv.empty()) {
                    serv = "80";
                    socket = make_http_socket(ec, resolver_.resolve(typename resolver_type::query(host, serv)), retry);
                } else {
                    socket = make_http_socket(ec, resolver_.resolve(typename resolver_type::query(host, serv)), retry);
                    host += ':';
                    host += serv;
                }
                break;
            case 2:
                if (!context_)
                    throw std::runtime_error("unsupported https");

                if (serv.empty()) {
                    serv = "443";
                    socket = make_https_socket(*context_, ec, resolver_.resolve(typename resolver_type::query(host, serv)), retry);
                } else {
                    socket = make_https_socket(*context_, ec, resolver_.resolve(typename resolver_type::query(host, serv)), retry);
                    host += ':';
                    host += serv;
                }
                break;
        }

        std::ostream & os = *socket;
        req.method(os);
        os << ' ';
        if (it == uri.end() || *it != '/')
            os << '/';
        std::copy(it, uri.end(), std::ostreambuf_iterator<char>(os));
        write_query(os, uri, (typename Uri::query_type *)(nullptr) );
        os << " "
            "HTTP/1.1\r\n"
            "Host: " << host << "\r\n";  // 3xxの移動系の情報必要なため、必ずヘッダーの最初に記述する
        write_header(os, uri, (typename Uri::header_type *)(nullptr));
        req.content(os);

        return socket;
    }

    template <typename Uri, typename T>
    void write_query(std::ostream & os, Uri const & uri, T const *)
    {
        uri.query(os);
    }

    template <typename Uri>
    void write_query(std::ostream &, Uri const &, boost::blank const *)
    {
    }

    template <typename Uri, typename T>
    void write_header(std::ostream & os, Uri const & uri, T const *)
    {
        uri.header(os);
    }

    template <typename Uri>
    void write_header(std::ostream & os, Uri const &, boost::blank const *)
    {
        os <<
            "Connection: " << (enabled_keep_alive_ ? "Keep-Alive" : "Close") << "\r\n"
            "Accept: */*\r\n"
            "Accept-Encoding: compress, gzip\r\n"
            ;
    }

    void lock_wait(bool mode)
    {
        std::unique_lock<decltype(mutex_)> lock(mutex_);
        if (mode) cond_.wait(lock, [this] { return (count_ + keep_sockets_.size()) < max_count_; });
        ++count_;
    }

    template <typename SocketPtr>
    void socket_deleter(SocketPtr * socket)
    {
        if (socket->is_keep_alive())
            collect(socket);
        else
            delete socket;

        std::lock_guard<decltype(mutex_)> lock(mutex_);
        --count_;
        cond_.notify_one();
    }

    template <typename SocketPtr>
    void collect(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        keep_sockets_.emplace(socket);
        socket->async_keep_alive();
    }

    template <typename SocketPtr>
    SocketPtr * reuse(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);
        auto it = keep_sockets_.find(socket);
        if (it != keep_sockets_.end()) {
            socket = static_cast<SocketPtr *>(*it);
            keep_sockets_.erase(it);
        } else {
            socket = nullptr;
        }

        return socket;
    }

    template <typename SocketPtr>
    void erase(SocketPtr * socket)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        for(auto it = keep_sockets_.find(socket); it != keep_sockets_.end(); ++it) {
            if (*it == socket) {
                keep_sockets_.erase(it);
                cond_.notify_one();
                return;
            }

            if (static_cast<SocketPtr const *>(*it)->endpoint() != socket->endpoint()) {
                break;
            }
        }
    }

    std::shared_ptr<socket_type> make_http_socket(boost::system::error_code const & error, typename resolver_type::iterator it, int retry)
    {
        std::shared_ptr<http_socket> http(new http_socket(this, resolver_.get_io_service(), retry), std::bind(&basic_http_client::socket_deleter<http_socket>, this, std::placeholders::_1));
        http->set_timeout(std::chrono::seconds(5));
        http->async_connect(error, it);
        return std::static_pointer_cast<socket_type>(http);
    }

    std::shared_ptr<socket_type> make_https_socket(boost::asio::ssl::context & ctx, boost::system::error_code const & error, typename resolver_type::iterator it, int retry)
    {
        std::shared_ptr<https_socket> https(new https_socket(this, resolver_.get_io_service(), ctx, retry), std::bind(&basic_http_client::socket_deleter<https_socket>, this, std::placeholders::_1));
        https->set_verify_peer();
        https->set_timeout(std::chrono::seconds(5));
        https->async_connect(error, it);
        return std::static_pointer_cast<socket_type>(https);
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
    boost::asio::ssl::context * context_;
    std::size_t max_count_;
    std::size_t count_;
    std::unordered_multiset<socket_type *, functor, functor> keep_sockets_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool enabled_keep_alive_ = true;
};


/*!
  HTTPクライアントのサービスクラス.
 */
using http_client = basic_http_client<client_result, boost::asio::steady_timer>;

} }
