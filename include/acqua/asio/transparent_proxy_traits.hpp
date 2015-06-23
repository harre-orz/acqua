#pragma once

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
}

#include <boost/asio/ip/tcp.hpp>
#include <acqua/asio/socket_base/transparent_options.hpp>
#include <acqua/asio/proxy_traits.hpp>

namespace acqua { namespace asio {

/*!
  透過プロキシサーバトレイト.
*/
template <typename T>
class transparent_proxy_traits
    : public proxy_traits<T>
{
public:
    template <typename Acceptor>
    static void set_option_v4(Acceptor & acc, boost::system::error_code &)
    {
        acc.set_option(socket_base::transparent_v4(true));
    }

    template <typename Acceptor>
    static void set_option_v6(Acceptor & acc, boost::system::error_code &)
    {
        acc.set_option(socket_base::transparent_v6(true));
    }

    void start_v4(T * t)
    {
        boost::system::error_code ec;
        auto & sv = t->server_socket();
        auto & cl = t->client_socket();
        cl.open(boost::asio::ip::tcp::v4(), ec);
        cl.set_option(socket_base::transparent_v4(true), ec);
        cl.bind(sv.local_endpoint(ec), ec);
        t->start();
    }

    void start_v6(T * t)
    {
        boost::system::error_code ec;
        auto & sv = t->server_socket();
        auto & cl = t->client_socket();
        cl.open(boost::asio::ip::tcp::v6(), ec);
        cl.set_option(socket_base::transparent_v6(true), ec);
        cl.bind(sv.local_endpoint(ec), ec);
        t->start();
    }
};

} }
