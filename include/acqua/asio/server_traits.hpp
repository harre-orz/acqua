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
    T * construct(boost::asio::io_service & io_service)
    {
        return new T(io_service);
    }

    /*!
      Acceptorクラスのオプションを指定するメソッドを記述します.
    */
    template <typename Acceptor>
    static void set_option(Acceptor &, boost::system::error_code &)
    {
    }

    /*!
      internet_server クラスで、IPv4用の Acceptor のオプションを設定するメソッドを記述します.
    */
    template <typename Acceptor>
    static void set_option_v4(Acceptor &, boost::system::error_code &)
    {
    }

    /*!
      internet_server クラスで、IPv6用の Acceptor のオプションを設定するメソッドを記述します.
    */
    template <typename Acceptor>
    static void set_option_v6(Acceptor &, boost::system::error_code &)
    {
    }
};

} }
