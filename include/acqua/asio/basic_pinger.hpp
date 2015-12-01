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

    void start()
    {
        boost::system::error_code ec;
        service_.start(ec);
        boost::asio::detail::throw_error(ec, "start");
    }

    void start(boost::system::error_code & ec)
    {
        service_.start(ec);
    }

    void cancel()
    {
        boost::system::error_code ec;
        service_.cancel(ec);
        boost::asio::detail::throw_error(ec, "cancel");
    }

    void cancel(boost::system::error_code & ec)
    {
        service_.cancel(ec);
    }

    void close()
    {
        boost::system::error_code ec;
        service_.shutdown(ec);
        boost::asio::detail::throw_error(ec, "shutdown");
    }

    void close(boost::system::error_code & ec)
    {
        service_.shutdown(ec);
    }

    template <typename Handler>
    void check(std::string const & host, time_point const & expire, Handler handler)
    {
        service_.ping_any(host, expire, handler);
    }

    template <typename Handler>
    void check(std::string const & host, duration const & expire, Handler handler)
    {
        check(host, timer_traits::now() + expire, handler);
    }


    template <typename Handler>
    void search(std::string const & host, time_point const & expire, Handler handler)
    {
        service_.ping_all(host, expire, handler);
    }

    template <typename Handler>
    void search(std::string const & host, duration const & expire, Handler handler)
    {
        search(host, timer_traits::now() + expire, handler);
    }

private:
    service_type service_;
};

} }
