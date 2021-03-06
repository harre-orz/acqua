/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/system/error_code.hpp>
#include <memory>

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
    template <typename Deleter, typename ... Args>
    static std::shared_ptr<T> construct(Deleter deleter, boost::asio::io_service & io_service, Args&&... args)
    {
        return std::shared_ptr<T>(new T(io_service, args...), deleter);
    }

    static void destruct(T * soc) noexcept
    {
        delete soc;
    }

    //! 接続済みソケットの最下位レイヤーを返す.
    static LowestLayerType & socket(std::shared_ptr<T> & soc)
    {
        return soc->socket();
    }

    //! クライアントと接続し、接続済みソケットの処理を開始.
    static void start(std::shared_ptr<T> & soc)
    {
        soc->start();
    }
};

} }
