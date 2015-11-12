/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/optional.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <acqua/asio/server_traits.hpp>
#include <acqua/asio/socket_category.hpp>
#include <acqua/asio/detail/simple_server_base.hpp>

namespace acqua { namespace asio {

/*!
  IPv4/v6 のどちらか/両方で Listen するサーバークラス
 */
template <
    typename Connector,
    typename Traits = server_traits<Connector>
    >
class internet_server
    : private Traits
    , private detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, internet_v4_tag>
    , private detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, internet_v6_tag>
{
    using v4_type = detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, internet_v4_tag>;
    friend v4_type;

    using v6_type = detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, internet_v6_tag>;
    friend v6_type;

    using atomic_size_type = typename v4_type::atomic_size_type;

public:
    using traits_type = Traits;
    using size_type = typename v4_type::size_type;
    using protocol_type = typename v4_type::protocol_type;
    using acceptor_type = typename v4_type::acceptor_type;
    using endpoint_type = typename v4_type::endpoint_type;

public:
    template <typename... Args>
    explicit internet_server(boost::asio::io_service & io_service, Args&&... args)
        : v4_type(io_service, args...)
        , v6_type(io_service, args...)
        , count_(0)
    {
    }

    size_type max_count() const
    {
        return max_count_;
    }

    void max_count(size_type count)
    {
        boost::system::error_code ec;
        max_count(count, ec);
        boost::asio::detail::throw_error(ec, "max_count");
    }

    void max_count(size_type count, boost::system::error_code & ec)
    {
        if (2 <= count)
            count_ = count;
        else
            ec = make_error_code(boost::system::errc::invalid_argument);
    }

    void listen(endpoint_type const & endpoint, bool reuse_addr = true)
    {
        boost::system::error_code ec;
        listen(endpoint, ec, reuse_addr);
        boost::asio::detail::throw_error(ec, "listen");
    }

    void listen(endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr = true)
    {
        if (endpoint.protocol() == protocol_type::v4())
            v4_type::listen(endpoint, ec, reuse_addr);
        if (endpoint.protocol() == protocol_type::v6())
            v6_type::listen(endpoint, ec, reuse_addr);
    }

    void listen(boost::asio::ip::address_v4 const & addr, std::uint16_t port, bool reuse_addr = true)
    {
        boost::system::error_code ec;
        listen(addr, port, ec, reuse_addr);
        boost::asio::detail::throw_error(ec, "listen");
    }

    void listen(boost::asio::ip::address_v4 const & addr, std::uint16_t port, boost::system::error_code & ec, bool reuse_addr = true)
    {
        v4_type::listen(endpoint_type(addr, port), ec, reuse_addr);
    }

    void listen(boost::asio::ip::address_v6 const & addr, std::uint16_t port, bool reuse_addr = true)
    {
        boost::system::error_code ec;
        listen(addr, port, ec, reuse_addr);
        boost::asio::detail::throw_error(ec, "listen");
    }

    void listen(boost::asio::ip::address_v6 const & addr, std::uint16_t port, boost::system::error_code & ec, bool reuse_addr = true)
    {
        v6_type::listen(endpoint_type(addr, port), ec, reuse_addr);
    }

    void listen(std::uint16_t port, bool reuse_addr = true)
    {
        boost::system::error_code ec;
        listen(port, ec, reuse_addr);
        boost::asio::detail::throw_error(ec, "listen");
    }

    void listen(std::uint16_t port, boost::system::error_code & ec, bool reuse_addr = true)
    {
        v4_type::listen(endpoint_type(boost::asio::ip::address_v4::any(), port), ec, reuse_addr);
        v6_type::listen(endpoint_type(boost::asio::ip::address_v6::any(), port), ec, reuse_addr);
    }

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec, "start");
    }

    void start(boost::system::error_code & ec)
    {
        if (!v4_type::get_acceptor().is_open()  && !v6_type::get_acceptor().is_open()) {
            ec = make_error_code(boost::asio::error::not_socket);
            return;
        }

        if (v4_type::get_acceptor().is_open())
            v4_type::start(ec);

        if (!ec && v6_type::get_acceptor().is_open()) {
            v6_type::start(ec);

            if (ec) {
                boost::system::error_code ec2;
                v4_type::cancel(ec2);
            }
        }
    }

    void cancel()
    {
        boost::system::error_code ec;
        cancel(ec);
        boost::asio::detail::throw_error(ec, "cancel");
    }

    void cancel(boost::system::error_code & ec)
    {
        boost::system::error_code ec2;

        if (v4_type::get_acceptor().is_open())
            v4_type::cancel(ec);

        if (v6_type::get_acceptor().is_open())
            v6_type::cancel(ec2);

        if (!ec && ec2) ec = ec2;
    }

private:
    using traits_type::set_option;
    using traits_type::construct;
    using traits_type::socket;
    using traits_type::start;

private:
    atomic_size_type count_;
    size_type max_count_ = 100;
};

} }
