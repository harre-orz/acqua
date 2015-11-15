#pragma once

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
}

#include <boost/asio/ip/tcp.hpp>
#include <acqua/asio/socket_category.hpp>
#include <acqua/asio/proxy_traits.hpp>
#include <acqua/asio/socket_base/transparent_options.hpp>

namespace acqua { namespace asio {

/*!
  透過プロキシサーバトレイト.
  Linux 限定
*/
template <typename T>
struct transparent_proxy_traits
    : public proxy_traits<T>
{
    template <typename Tag, typename Socket>
    static void set_option(Tag, Socket & soc, boost::asio::ip::tcp const & proto, boost::system::error_code & ec)
    {
        if (proto == transparent_proxy_traits::protocol_type::v4())
            soc.set_option(socket_base::transparent_v4(true), ec);
        if (proto == transparent_proxy_traits::protocol_type::v6())
            soc.set_option(socket_base::transparent_v6(true), ec);
    }

    template <typename Socket>
    static void set_option(internet_v4_tag, Socket & soc, boost::asio::ip::tcp const &, boost::system::error_code & ec)
    {
        soc.set_option(socket_base::transparent_v4(true), ec);
    }

    template <typename Socket>
    static void set_option(internet_v6_tag, Socket & soc, boost::asio::ip::tcp const &, boost::system::error_code & ec)
    {
        soc.set_option(socket_base::transparent_v6(true), ec);
    }

    template <typename Tag>
    static void start(Tag tag, std::shared_ptr<T> soc)
    {
        auto & sv = soc->server_socket();
        auto & cl = soc->client_socket();

        boost::system::error_code ec;
        auto ep = sv.local_endpoint(ec);
        if (ec) return;

        auto proto = ep.protocol();
        cl.open(proto, ec);
        if (ec) return;

        set_option(tag, cl, proto, ec);
        if (ec) return;

        cl.bind(ep, ec);
        if (ec) return;

        soc->start();
    }
};

} }
