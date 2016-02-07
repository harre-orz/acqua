/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <acqua/asio/detail/simple_server_base.hpp>
#include <acqua/asio/server_traits.hpp>
#include <boost/asio/io_service.hpp>

namespace acqua { namespace asio {

/*!
  accept がある度に Connector クラスを生成するシンプルなサーバクラス.
  \tparam Connector std::enable_shared_from_this を継承していること
  \tparam Protocol boost::asio::ip::tcp もしくは boost::asio::local::stream_protocol
*/
template <
    typename Connector,
    typename Traits = server_traits<Connector>,
    typename Protocol = typename Connector::protocol_type
    >
class simple_server
    : private Traits
    , private detail::simple_server_base<simple_server<Connector, Traits, Protocol>, Connector, Protocol>
{
    using base_type = typename simple_server::base_type;
    friend base_type;

    using atomic_size_type = typename base_type::atomic_size_type;

public:
    using traits_type = Traits;
    using size_type = typename base_type::size_type;
    using protocol_type = typename base_type::protocol_type;
    using acceptor_type = typename base_type::acceptor_type;
    using endpoint_type = typename base_type::endpoint_type;

public:
    template <typename... Args>
    explicit simple_server(boost::asio::io_service & io_service, Args... args)
        : base_type(io_service, args...)
        , use_count_(0)
    {
    }

    size_type use_count() const
    {
        return use_count_;
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
        if (1 <= count)
            max_count_ = count;
        else
            ec = make_error_code(boost::system::errc::invalid_argument);
    }

    void listen(endpoint_type const & endpoint, bool reuse_addr = true)
    {
        boost::system::error_code ec;
        listen(endpoint, ec, reuse_addr);
        boost::asio::detail::throw_error(ec, "listen");
    }

    using base_type::listen;

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec);
    }

    using base_type::start;

    void cancel()
    {
        boost::system::error_code ec;
        cancel(ec);
        boost::asio::detail::throw_error(ec);
    }

    using base_type::cancel;

    void close()
    {
        boost::system::error_code ec;
        close(ec);
        boost::asio::detail::throw_error(ec);
    }

    using base_type::close;

private:
    using traits_type::set_option;
    using traits_type::construct;
    using traits_type::destruct;
    using traits_type::socket;
    using traits_type::start;

private:
    atomic_size_type use_count_;
    size_type max_count_ = 100;
};

} }
