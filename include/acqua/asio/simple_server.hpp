/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/io_service.hpp>
#include <acqua/exception/throw_error.hpp>
#include <acqua/asio/detail/simple_server_base.hpp>
#include <acqua/asio/server_traits.hpp>

namespace acqua { namespace asio {

/*!
 accept がある度に Connector クラスを生成して、また accept が来るまで待つだけのシンプルなサーバ.

 boost::asio::ip::tcp::acceptor もしくは boost::asio::local::stream_protocol::acceptor の待受ソケットを使い、コネクションクラスを自動生成することができる

 \tparam Connector std::shared_ptr で管理されるので、Connector クラスは必ず std::enable_shared_from_this を継承していなければならない。

 */
template <
    typename Connector,
    typename Protocol = typename Connector::protocol_type,
    typename Traits = server_traits<Connector>
    >
class simple_server
    : private Traits
    , private detail::simple_server_base<simple_server<Connector, Protocol, Traits>, Connector, Protocol>

{
    using base_type = detail::simple_server_base<simple_server<Connector, Protocol, Traits>, Connector, Protocol>;
    friend base_type;

public:
    using traits_type = Traits;
    using protocol_type = typename base_type::protocol_type;
    using acceptor_type = typename base_type::acceptor_type;
    using endpoint_type = typename base_type::endpoint_type;

public:
    explicit simple_server(boost::asio::io_service & io_service, endpoint_type const & endpoint, std::size_t max_count = 100, Traits traits = Traits(), bool reuse_addr = true)
        : traits_type(std::move(traits))
        , base_type(io_service, count_)
        , max_count_(max_count)
        , count_(0)
    {
        boost::system::error_code ec;
        if (max_count_ <= 0)
            throw boost::system::system_error(EINVAL, boost::system::generic_category());
        listen(base_type::acceptor(), endpoint, ec, reuse_addr);
    }

    void start()
    {
        base_type::start();
    }

    void stop()
    {
        base_type::stop();
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
    void listen(acceptor_type & acc, endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr)
    {
        acc.open(endpoint.protocol(), ec);
       acqua::exception::throw_error(ec, "open");
        if (reuse_addr) {
            acc.set_option(boost::asio::socket_base::reuse_address(true), ec);
           acqua::exception::throw_error(ec, "set_option");
        }
        static_cast<traits_type *>(this)->set_option(acc, ec);
        acc.bind(endpoint, ec);
       acqua::exception::throw_error(ec, "bind");
        acc.listen(boost::asio::socket_base::max_connections, ec);
       acqua::exception::throw_error(ec, "listen");
    }

    Connector * construct(boost::asio::io_service & io_service)
    {
        return static_cast<Traits *>(this)->construct(io_service);
    }

    typename Protocol::socket & server_socket(std::shared_ptr<Connector> & conn, boost::blank const &)
    {
        return static_cast<Traits *>(this)->template socket<typename Protocol::socket>(&*conn);
    }

    void connection_start(std::shared_ptr<Connector> & conn, boost::blank const &)
    {
        static_cast<Traits *>(this)->start(&*conn);
    }

private:
    std::size_t max_count_;
    std::atomic<std::size_t> count_;
};

} }
