/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>
#include <boost/asio/io_service.hpp>

namespace acqua { namespace asio {

/*!
  simple_server クラス, internet_server クラスに用いる特性を記述するクラス.
 */
template <typename T>
class server_traits
{
public:
    //! サーバソケットを設定.
    template <typename Tag, typename Acceptor>
    void set_option(Tag, Acceptor & acc, boost::system::error_code & ec, bool reuse_addr)
    {
        if (reuse_addr) {
            acc.set_option(boost::asio::socket_base::reuse_address(true), ec);
        }
    }

    //! 接続済みソケットのコンテキストを作成.
    //! 第二引数以降は、サーバサービスのコンストラクタ第二引数以降に対応している
    template <typename... Args>
    static T * construct( boost::asio::io_service & io_service, Args&&... args)
    {
        return new T(io_service, args...);
    }

    //! 接続済みソケットの最下位レイヤーを返す.
    template <typename Tag, typename SocketPtr>
    typename SocketPtr::element_type::lowest_layer_type & socket(Tag, SocketPtr soc)
    {
        return soc->socket();
    }

    //! クライアントと接続し、接続済みソケットの処理を開始.
    template <typename Tag, typename SocketPtr>
    static void start(Tag, SocketPtr soc)
    {
        soc->start();
    }
};

} }
