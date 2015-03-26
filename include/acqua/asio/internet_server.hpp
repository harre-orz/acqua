/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/optional.hpp>

#include <acqua/exception/throw_error.hpp>
#include <acqua/asio/server_traits.hpp>
#include <acqua/asio/detail/internet_tag.hpp>
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
    , private detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, detail::internet_v4_tag>
    , private detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, detail::internet_v6_tag>
{
    using base_v4_type = detail::simple_server_base<internet_server<Connector, Traits>, Connector,boost::asio::ip::tcp, detail::internet_v4_tag>;
    friend base_v4_type;

    using base_v6_type = detail::simple_server_base<internet_server<Connector, Traits>, Connector, boost::asio::ip::tcp, detail::internet_v6_tag>;
    friend base_v6_type;

public:
    using traits_type = Traits;
    using protocol_type = typename base_v4_type::protocol_type;
    using acceptor_type = typename base_v4_type::acceptor_type;
    using endpoint_type =  typename base_v4_type::endpoint_type;

public:
    explicit internet_server(boost::asio::io_service & io_service, boost::optional<boost::asio::ip::address> const & address, std::uint16_t port, std::size_t max_count = 100, Traits traits = Traits(), bool reuse_addr = true)
        : traits_type(std::move(traits))
        , base_v4_type(io_service, count_)
        , base_v6_type(io_service, count_)
        , count_(0)
    {
        boost::system::error_code ec;
        if (!address) {
            set_max_count(max_count, 2, ec);
            listen_v4(base_v4_type::acceptor(), endpoint_type(boost::asio::ip::address_v4::any(), port), ec, reuse_addr);
            listen_v6(base_v6_type::acceptor(), endpoint_type(boost::asio::ip::address_v6::any(), port), ec, reuse_addr);
        } else if (address->is_v4()) {
            set_max_count(max_count, 1, ec);
            listen_v4(base_v4_type::acceptor(), endpoint_type(*address, port), ec, reuse_addr);
        } else if (address->is_v6()) {
            set_max_count(max_count, 1, ec);
            listen_v6(base_v6_type::acceptor(), endpoint_type(*address, port), ec, reuse_addr);
        }
    }

    explicit internet_server(boost::asio::io_service & io_service, endpoint_type const & endpoint, std::size_t max_count = 100, Traits traits = Traits(), bool reuse_addr = true)
        : Traits(std::move(traits))
        , base_v4_type(io_service, count_)
        , base_v6_type(io_service, count_)
        , count_(0)
    {
        boost::system::error_code ec;
        if (endpoint.protocol() == boost::asio::ip::tcp::v4()) {
            set_max_count(max_count, 1, ec);
            listen_v4(base_v4_type::acceptor(), endpoint, ec, reuse_addr);
        } else if (endpoint.protocol() == boost::asio::ip::tcp::v6()) {
            set_max_count(max_count, 1, ec);
            listen_v6(base_v6_type::acceptor(), endpoint, ec, reuse_addr);
        }
    }

    explicit internet_server(boost::asio::io_service & io_service, std::uint16_t port, std::size_t max_count = 100, Traits traits = Traits(), bool reuse_addr = true)
        : Traits(std::move(traits))
        , base_v4_type(io_service, count_)
        , base_v6_type(io_service, count_)
        , count_(0)
    {
        boost::system::error_code ec;
        set_max_count(max_count, 2, ec);
        listen_v4(base_v4_type::acceptor(), endpoint_type(boost::asio::ip::address_v4::any(), port), ec, reuse_addr);
        listen_v6(base_v6_type::acceptor(), endpoint_type(boost::asio::ip::address_v6::any(), port), ec, reuse_addr);
    }

    void start()
    {
        base_v4_type::start();
        base_v6_type::start();
    }

    void stop()
    {
        base_v4_type::stop();
        base_v6_type::stop();
    }

    std::size_t use_count() const noexcept
    {
        return count_;
    }

    std::size_t max_count() const noexcept
    {
        return max_count_;
    }

private:
    void listen_v4(acceptor_type & acc, endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr)
    {
        acc.open(endpoint.protocol(), ec);
       acqua::exception::throw_error(ec, "open");
        if (reuse_addr) {
            acc.set_option(boost::asio::socket_base::reuse_address(true), ec);
           acqua::exception::throw_error(ec, "set_option");
        }
        static_cast<traits_type *>(this)->set_option(acc, ec);
        static_cast<traits_type *>(this)->set_option_v4(acc, ec);
        acc.bind(endpoint, ec);
       acqua::exception::throw_error(ec, "bind");
        acc.listen(boost::asio::socket_base::max_connections, ec);
       acqua::exception::throw_error(ec, "listen");
    }

    void listen_v6(acceptor_type & acc, endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr)
    {
        acc.open(endpoint.protocol(), ec);
       acqua::exception::throw_error(ec, "open");
        if (reuse_addr) {
            acc.set_option(boost::asio::socket_base::reuse_address(true), ec);
           acqua::exception::throw_error(ec, "set_option");
        }
        acc.set_option(boost::asio::ip::v6_only(true));
       acqua::exception::throw_error(ec, "set_option");
        static_cast<traits_type *>(this)->set_option(acc, ec);
        static_cast<traits_type *>(this)->set_option_v6(acc, ec);
        acc.bind(endpoint, ec);
       acqua::exception::throw_error(ec, "bind");
        acc.listen(boost::asio::socket_base::max_connections, ec);
       acqua::exception::throw_error(ec, "listen");
    }

    Connector * construct(boost::asio::io_service & io_service)
    {
        return static_cast<Traits *>(this)->construct(io_service);
    }

    void set_max_count(std::size_t max_count, std::size_t lower_limit, boost::system::error_code & ec)
    {
        if (max_count < lower_limit) {
            ec = boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
            acqua::exception::throw_error(ec, "max_count");
        }

        max_count_ = max_count;
    }

private:
    std::atomic<std::size_t> count_;
    std::size_t max_count_;
};

} }
