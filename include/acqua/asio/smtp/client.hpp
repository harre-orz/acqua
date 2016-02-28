#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>

namespace acqua { namespace asio { namespace smtp {

/*!
  SMTPクライアントクラス.

  各SMTPプロトコルメソッドは、例外版と非例外版が用意されている。
  また、各戻り値は、SMTPプロトコルにおける応答コードになるが、通信エラーの場合は 0 になる。

  Supported STARTTLS ...ok
  Supported AUTH PLAIN LOGIN CRAM-MD5 ...ok
 */
class client
{
    struct impl;

public:
    explicit client(boost::asio::io_service & io_service);

    /*!
      非暗号化プロトコルでサーバに接続する.
     */
    uint connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv);

    /*!
      非暗号化プロトコルでサーバに接続する.
     */
    uint connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv, boost::system::error_code & ec);

    /*!
      (未実装) 暗号化プロトコルでサーバに接続する.
     */
    uint connect_ssl(boost::asio::yield_context yield, std::string const & host, std::string const & serv,
                     std::string const & keyfile = "", std::string const & certfile = "");

    /*!
      (未実装) 暗号化プロトコルでサーバに接続する.
    */
    uint connect_ssl(boost::asio::yield_context yield, std::string const & host, std::string const & serv, boost::system::error_code & ec,
                     std::string const & keyfile = "", std::string const & certfile = "");

    /*!
      HELO コマンドを行う.
      なるべく ehlo() を用いること
     */
    uint helo(boost::asio::yield_context yield, std::string const & hostname = "");

    /*!
      HELO コマンドを行う.
      なるべく ehlo() を用いること
    */
    uint helo(boost::asio::yield_context yield, boost::system::error_code & ec, std::string const & hostname = "");

    /*!
      EHLO コマンドを行う.
      STARTTLS が利用可能であれば、自動的に切り替わる
    */
    uint ehlo(boost::asio::yield_context yield, std::string const & hostname = "");

    /*!
      EHLO コマンドを送信する.
      STARTTLS が利用可能であれば、自動的に切り替わる
    */
    uint ehlo(boost::asio::yield_context yield, boost::system::error_code & ec, std::string const & hostname = "");

    /*!
      サーバが利用可能な認証プロトコルを用いて、認証を行う.
     */
    uint login(boost::asio::yield_context yield, std::string const & user, std::string const & pass);

    /*!
      サーバが利用可能な認証プロトコルを用いて、認証を行う.
     */
    uint login(boost::asio::yield_context yield, std::string const & user, std::string const & pass, boost::system::error_code & ec);

    /*!
      MAIL コマンドを行う.
     */
    uint mail(boost::asio::yield_context yield, std::string const & address);

    /*!
      MAIL コマンドを行う.
     */
    uint mail(boost::asio::yield_context yield, std::string const & address, boost::system::error_code & ec);

    /*!
      RCPT コマンドを行う.
     */
    uint rcpt(boost::asio::yield_context yield, std::string const & address);

    /*!
      RCPT コマンドを行う.
     */
    uint rcpt(boost::asio::yield_context yield, std::string const & address, boost::system::error_code & ec);

    /*!
      DATA コマンドを行い、メール内容を転送する.
      is の最後は必ず ".\r\n" で終わらなければならない
     */
    uint data(boost::asio::yield_context yield, std::istream & is);

    /*!
      DATA コマンドを行う.
      is の最後は必ず ".\r\n" で終わらなければならない
     */
    uint data(boost::asio::yield_context yield, std::istream & is, boost::system::error_code & ec);

    /*!
      QUIT コマンドを行い、サーバとの接続を切る.
     */
    uint quit(boost::asio::yield_context yield);

    /*!
      QUIT コマンドを行い、サーバとの接続を切る.
     */
    uint quit(boost::asio::yield_context yield, boost::system::error_code & ec);

    /*!
      通信のやり取りを os に出力する.
      verbose が true のときは、本文も出力する
     */
    void dump_socketstream(std::ostream & os, bool verbose = false);

    /*!
      サーバ認証に平文パスワードを使用しない.
     */
    void disable_plaintext_password();

    /*!
      STARTTLS を使用しない.
     */
    void disable_starttls();

private:
    std::unique_ptr<impl> impl_;
};

} } }

#include <acqua/asio/smtp/client.ipp>
