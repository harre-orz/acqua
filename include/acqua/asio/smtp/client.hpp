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

class client
{
    struct impl;

public:
    explicit client(boost::asio::io_service & io_service);

    template <typename Handler>
    void connect(boost::asio::basic_yield_context<Handler> yield,
                 std::string const & host, std::string const & serv);

    template <typename Handler>
    uint connect(boost::asio::basic_yield_context<Handler> yield,
                 std::string const & host, std::string const & serv, boost::system::error_code & ec);

    template <typename Handler>
    void connect_ssl(boost::asio::basic_yield_context<Handler> yield,
                     std::string const & host, std::string const & serv,
                     std::string const & keyfile = "", std::string const & certfile = "");

    template <typename Handler>
    uint connect_ssl(boost::asio::basic_yield_context<Handler> yield,
                     std::string const & host, std::string const & serv, boost::system::error_code & ec,
                     std::string const & keyfile = "", std::string const & certfile = "");

    template <typename Handler>
    void helo(boost::asio::basic_yield_context<Handler> yield,
              std::string const & hostname = "");

    template <typename Handler>
    uint helo(boost::asio::basic_yield_context<Handler> yield,
              boost::system::error_code & ec, std::string const & hostname = "");

    template <typename Handler>
    void ehlo(boost::asio::basic_yield_context<Handler> yield,
              std::string const & hostname = "");

    template <typename Handler>
    uint ehlo(boost::asio::basic_yield_context<Handler> yield,
              boost::system::error_code & ec, std::string const & hostname = "");

    template <typename Handler>
    void login(boost::asio::basic_yield_context<Handler> yield,
               std::string const & user, std::string const & pass);

    template <typename Handler>
    uint login(boost::asio::basic_yield_context<Handler> yield,
               std::string const & user, std::string const & pass, boost::system::error_code & ec);

    template <typename Handler>
    void mail(boost::asio::basic_yield_context<Handler> yield,
              std::string const & address);

    template <typename Handler>
    uint mail(boost::asio::basic_yield_context<Handler> yield,
               std::string const & address, boost::system::error_code & ec);

    template <typename Handler>
    void rcpt(boost::asio::basic_yield_context<Handler> yield,
              std::string const & address);

    template <typename Handler>
    uint rcpt(boost::asio::basic_yield_context<Handler> yield,
              std::string const & address, boost::system::error_code & ec);

    template <typename Handler>
    void data(boost::asio::basic_yield_context<Handler> yield,
              std::istream & is);

    template <typename Handler>
    uint data(boost::asio::basic_yield_context<Handler> yield,
              std::istream & is, boost::system::error_code & ec);

    template <typename Handler>
    void quit(boost::asio::basic_yield_context<Handler> yield);

    template <typename Handler>
    uint quit(boost::asio::basic_yield_context<Handler> yield,
              boost::system::error_code & ec);

    /*!
      通信のやり取りを os に出力する.
      verbose が true のときは、本文も出力する
     */
    void dump_socketstream(std::ostream & os, bool verbose = false);

    /*!
      サーバ認証に平文パスワードの使用を許可する.
     */
    void enable_plaintext_password();

    /*!
      STARTSSL が有効であれば使用する.
     */
    void enable_startssl();

private:
    std::unique_ptr<impl> impl_;
};

} } }

#include <acqua/asio/smtp/client.ipp>
