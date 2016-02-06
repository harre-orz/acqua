/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/asio/socket_base/transparent_options.hpp>
#include <acqua/asio/proxy_traits.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <memory>

extern "C" {
#include <sys/types.h>
#include <sys/socket.h>
}

namespace acqua { namespace asio {

/*!
  透過プロキシサーバトレイト.
  Linux 限定
*/
template <typename T, typename LowestLayerType = typename T::lowest_layer_type>
struct transparent_proxy_traits
    : public proxy_traits<T, LowestLayerType>
{
    template <typename Socket>
    static void set_option(Socket & soc, boost::asio::ip::tcp const & proto, boost::system::error_code & ec)
    {
        if (proto == transparent_proxy_traits::protocol_type::v4())
            soc.set_option(socket_base::transparent_v4(true), ec);
        if (proto == transparent_proxy_traits::protocol_type::v6())
            soc.set_option(socket_base::transparent_v6(true), ec);
    }

    static void start(std::shared_ptr<T> & soc)
    {
        auto & sv = soc->server_socket();
        auto & cl = soc->client_socket();

        boost::system::error_code ec;
        auto ep = sv.local_endpoint(ec);
        if (ec) return;

        auto proto = ep.protocol();
        cl.open(proto, ec);
        if (ec) return;

        set_option(cl, proto, ec);
        if (ec) return;

        cl.bind(ep, ec);
        if (ec) return;

        soc->start();
    }
};

} }
