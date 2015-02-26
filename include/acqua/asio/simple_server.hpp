#pragma once

#include <boost/asio/io_service.hpp>

#include <acqua/asio/detail/simple_server_base.hpp>
#include <acqua/asio/server_traits.hpp>

namespace acqua { namespace asio {

/*!
 accept がある度に Connector クラスを生成して、また accept が来るまで待つだけのシンプルなサーバ.

 Connector クラスは、std::shared_ptr で管理されるので、Connector クラスは必ず std::enable_shared_from_this を継承していること。(コンパイルエラーになる)
 tcp socket だけでなく、unix-domain socket を使用することも可能。
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
    typedef detail::simple_server_base<simple_server<Connector, Protocol, Traits>, Connector, Protocol> base_type;
    friend base_type;

public:
    typedef Traits traits_type;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::protocol_type protocol_type;
    typedef typename base_type::acceptor_type acceptor_type;
    typedef typename base_type::endpoint_type endpoint_type;

public:
    explicit simple_server(boost::asio::io_service & io_service, endpoint_type const & endpoint, bool volatile & marked_alive, size_type max_count = 100, Traits traits = Traits(), bool reuse_addr = true)
        : traits_type(std::move(traits))
        , base_type(io_service, marked_alive, count_)
        , count_(0)
        , max_count_(max_count)
    {
        boost::system::error_code ec;
        if (max_count_ < 1)
            ec = boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
        boost::asio::detail::throw_error(ec, "max_count");
        listen(base_type::acceptor(), endpoint, ec, reuse_addr);
    }

    using base_type::start;
    using base_type::stop;

    size_type use_count() const noexcept
    {
        return count_;
    }

    size_type max_count() const noexcept
    {
        return max_count_;
    }

private:
    void listen(acceptor_type & acc, endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr)
    {
        acc.open(endpoint.protocol(), ec);
        boost::asio::detail::throw_error(ec, "open");
        if (reuse_addr) {
            acc.set_option(boost::asio::socket_base::reuse_address(true), ec);
            boost::asio::detail::throw_error(ec, "set_option");
        }
        static_cast<traits_type *>(this)->set_option(acc, ec);
        acc.bind(endpoint, ec);
        boost::asio::detail::throw_error(ec, "bind");
        acc.listen(boost::asio::socket_base::max_connections, ec);
        boost::asio::detail::throw_error(ec, "listen");
    }

    Connector * construct(boost::asio::io_service & io_service)
    {
        return static_cast<Traits *>(this)->construct(io_service);
    }

private:
    std::atomic<size_type> count_;
    std::size_t max_count_;
};

} }
