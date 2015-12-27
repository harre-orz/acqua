/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <memory>
#include <boost/system/error_code.hpp>
#include <boost/asio/io_service.hpp>

namespace acqua { namespace asio {

/*!
  simple_server クラス, internet_server クラスに用いる特性を記述するクラス.
 */
template <typename T, typename LowestLayerType = typename T::lowest_layer_type>
struct server_traits
{
    //! サーバソケットを設定.
    template <typename Socket, typename Protocol>
    static void set_option(Socket &, Protocol const &, boost::system::error_code &)
    {
    }

    //! 接続済みソケットのコンテキストを作成.
    //! 第二引数以降は、サーバサービスのコンストラクタ第二引数以降に対応している
    template <typename... Args>
    static T * construct( boost::asio::io_service & io_service, Args&&... args)
    {
        return new T(io_service, args...);
    }

    static void destruct(T * soc)
    {
        delete soc;
    }

    //! 接続済みソケットの最下位レイヤーを返す.
    static LowestLayerType & socket(std::shared_ptr<T> soc)
    {
        return soc->socket();
    }

    //! クライアントと接続し、接続済みソケットの処理を開始.
    static void start(std::shared_ptr<T> soc)
    {
        soc->start();
    }
};

} }
