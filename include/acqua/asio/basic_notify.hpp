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
#include <boost/asio/detail/throw_error.hpp>

namespace acqua { namespace asio {

template <typename Service>
class basic_notify
    : public boost::asio::basic_io_object<Service>
{
public:
    using native_handle_type = typename Service::native_handle_type;
    using event = typename Service::event_type;
    using iterator = typename Service::iterator_type;
    using event_flags = typename Service::event_flags;

public:
    explicit basic_notify(boost::asio::io_service & io_service, bool with_open = true)
        : boost::asio::basic_io_object<Service>(io_service)
    {
        if (with_open)
            open();
    }

    void open()
    {
        boost::system::error_code ec;
        this->get_service().open(this->get_implementation(), ec);
        boost::asio::detail::throw_error(ec, "open");
    }

    boost::system::error_code open(boost::system::error_code & ec)
    {
        return this->get_service().open(this->get_implementation(), ec);
    }

    bool is_open() const
    {
        return this->get_service().is_open(this->get_implementation());
    }

    void assign(native_handle_type & handle)
    {
        boost::system::error_code ec;
        this->get_service().assign(this->get_implementation(), handle, ec);
        boost::asio::detail::throw_error(ec, "assign");
    }

    boost::system::error_code assign(native_handle_type & handle, boost::system::error_code & ec)
    {
        return this->get_service().assign(this->get_implementation(), handle, ec);
    }

    void close()
    {
        boost::system::error_code ec;
        this->get_service().close(this->get_implementation(), ec);
        boost::asio::detail::throw_error(ec, "close");
    }

    boost::system::error_code close(boost::system::error_code & ec)
    {
        return this->get_service().close(this->get_implementation(), ec);
    }

    native_handle_type native_handle() const
    {
        return this->get_service().native_handle(this->get_implementation());
    }

    void cancel()
    {
        boost::system::error_code ec;
        this->get_service().cancel(this->get_implementation(), ec);
        boost::asio::detail::throw_error(ec, "cancel");
    }

    boost::system::error_code cancel(boost::system::error_code & ec)
    {
        return this->get_service().cancel(this->get_implementation(), ec);
    }

    template <typename Handler>
    iterator wait(Handler handler)
    {
        boost::system::error_code ec;
        iterator it = this->get_service().wait(this->get_implementation(), handler, ec);
        boost::asio::detail::throw_error(ec, "wait");
        return it;
    }

    template <typename Handler>
    iterator wait(Handler handler, boost::system::error_code & ec)
    {
        return this->get_service().wait(this->get_implementation(), handler, ec);
    }

    template <typename Handler>
    void async_wait(Handler handler)
    {
        this->get_service().async_wait(this->get_implementation(), handler);
    }

    void add(std::string const & path, event_flags flags = Service::all_events)
    {
        boost::system::error_code ec;
        this->get_service().add(this->get_implementation(), path, flags, ec);
        boost::asio::detail::throw_error(ec, "add");
    }

    boost::system::error_code add(std::string const & path, boost::system::error_code & ec)
    {
        return this->get_service().add(this->get_implementation(), path, event_flags::all_events, ec);
    }

    boost::system::error_code add(std::string const & path, event_flags flags, boost::system::error_code & ec)
    {
        return this->get_service().add(this->get_implementation(), path, flags, ec);
    }

    void remove(std::string const & path)
    {
        boost::system::error_code ec;
        this->get_service().remove(this->get_implementation(), path, ec);
        boost::asio::detail::throw_error(ec, "remove");
    }

    boost::system::error_code remove(std::string const & path, boost::system::error_code & ec)
    {
        return this->get_service().remove(this->get_implementation(), path, ec);
    }

    bool exists(std::string const & path) const
    {
        return this->get_service().exists(this->get_implementation(), path);
    }

};

} }
