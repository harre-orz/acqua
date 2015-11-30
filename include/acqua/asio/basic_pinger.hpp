/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/ip/address.hpp>
#include <acqua/asio/timer_traits.hpp>

namespace acqua { namespace asio {

template <typename PingerService>
class basic_pinger
{
public:
    using service_type = PingerService;

private:
    using timer_traits = timer_traits<typename service_type::timer_type>;

public:
    using time_point = typename timer_traits::time_point;
    using duration = typename timer_traits::duration;

public:
    explicit basic_pinger(boost::asio::io_service & io_service)
        : service_(io_service, ::getpid())
    {
    }

    explicit basic_pinger(boost::asio::io_service & io_service, int id)
        : service_(io_service, id)
    {
    }

    /*!
      ICMPソケットをオープンし、送受信できる状態にする.
      stop() 済みで shutdown() が未処理のときも、送受信処理を再開することができる
     */
    void start()
    {
        boost::system::error_code ec;
        service_.start(ec);
        boost::asio::detail::throw_error(ec, "start");
    }

    /*!
      ICMPソケットをオープンし、送受信できる状態にする.
      stop() 済みで shutdown() が未処理のときも、送受信処理を再開することができる
     */
    void start(boost::system::error_code & ec)
    {
        service_.start(ec);
    }

    /*!
      送受信処理を中断する.
      応答待ちのハンドラはキャンセル状態でコールバックを返す
      ICMPソケットはクローズせず、start() 関数で再開することができる
     */
    void cancel()
    {
        boost::system::error_code ec;
        service_.cancel(ec);
        boost::asio::detail::throw_error(ec, "cancel");
    }

    /*!
      送受信処理を中断する.
      応答待ちのハンドラはキャンセル状態でコールバックを返す
      ICMPソケットはクローズせず、start() 関数で再開することができる
     */
    void cancel(boost::system::error_code & ec)
    {
        service_.cancel(ec);
    }

    /*!
      ICMPソケットをクローズする.
      応答待ちのハンドラ operation_cancel 状態でコールバックを返す
      start() 関数で再開することができる
     */
    void close()
    {
        boost::system::error_code ec;
        service_.shutdown(ec);
        boost::asio::detail::throw_error(ec, "shutdown");
    }

    /*!
      ICMPソケットをクローズする.
      応答待ちのハンドラはキャンセル状態でコールバックを返す
      start() 関数で再開することができる
     */
    void close(boost::system::error_code & ec)
    {
        service_.shutdown(ec);
    }

    /*!
      対象 host を非同期で ping の応答を確認する.
      Handlerの型は void (*)(boost::system::error_code const & error, boost::asio::ip::icmp::endpoint const & host) となっており、
      ping の応答があった場合 !error で host に対象のIPアドレス等がコールバック関数で応答する
      host の名前解決ができない場合や expire 時間までに応答がない場合 error のコールバック関数で応答する
    */
    template <typename Handler>
    void ping_host(std::string const & host, time_point const & expire, Handler handler)
    {
        service_.ping_host(host, expire, handler);
    }

    /*!
      対象 host を非同期で ping の応答を確認する.
      Handlerの型は void (*)(boost::system::error_code const & error, boost::asio::ip::icmp::endpoint const & host) となっており、
      ping の応答があった場合 !error で host に対象のIPアドレス等がコールバック関数で応答する
      host の名前解決ができない場合や expire 経過するまでに応答がない場合 error のコールバック関数で応答する
    */
    template <typename Handler>
    void ping_host(std::string const & host, duration const & expire, Handler handler)
    {
        service_.ping_host(host, timer_traits::now() + expire, handler);
    }

private:
    service_type service_;
};

} }
