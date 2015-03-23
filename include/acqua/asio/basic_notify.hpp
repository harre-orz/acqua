/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <boost/system/error_code.hpp>
#include <boost/asio/basic_io_object.hpp>


namespace acqua { namespace asio {

template <typename Service>
class basic_notify
    : public boost::asio::basic_io_object<Service>
{
public:
    using event = typename Service::event;
    using iterator = typename Service::iterator;

    explicit basic_notify(boost::asio::io_service & io_service)
        : boost::asio::basic_io_object<Service>(io_service)
    {
    }

    void cancel()
    {
        this->service.cancel(this->implementation);
    }

    void add(std::string const & filename, int flags, boost::system::error_code & ec)
    {
        this->service.add(this->implementation, filename, flags, ec);
    }

    void add(std::string const & filename)
    {
        boost::system::error_code ec;
        this->service.add(this->implementation, filename, event::all_events, ec);
        acqua::exception::throw_error(ec, "inotify_add_watch");
    }

    void remove(std::string const & filename, boost::system::error_code & ec)
    {
        this->service.remove(this->implementation, filename, ec);
    }

    template <typename Handler>
    void async_notify(Handler handler)
    {
        this->service.async_notify(this->implementation, handler);
    }
};

} }
