#pragma once

extern "C" {
#include <termios.h>
#include <unistd.h>
}

#include <boost/asio/error.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/detail/descriptor_ops.hpp>
#include <boost/asio/detail/reactive_descriptor_service.hpp>

namespace acqua { namespace asio {

class pseudo_terminal_service
    : public boost::asio::detail::service_base<pseudo_terminal_service>
{
public:
    static boost::asio::io_service::id id;

    using native_handle_type = boost::asio::detail::reactive_descriptor_service::native_handle_type;
    using implementation_type = boost::asio::detail::reactive_descriptor_service::implementation_type;

    pseudo_terminal_service(boost::asio::io_service & io_service)
        : boost::asio::detail::service_base<pseudo_terminal_service>(io_service)
        , descriptor_service_(io_service)
    {
    }

    void shutdown_service()
    {
        descriptor_service_.shutdown_service();
    }

    void construct(implementation_type & impl)
    {
        descriptor_service_.construct(impl);
    }

    void move_construct(implementation_type & impl, implementation_type & other_impl)
    {
        descriptor_service_.move_construct(impl, other_impl);
    }

    void move_assign(implementation_type & impl, pseudo_terminal_service & other_service, implementation_type & other_impl)
    {
        descriptor_service_.move_assign(impl, other_service.descriptor_service_, other_impl);
    }

    void destroy(implementation_type & impl)
    {
        descriptor_service_.destroy(impl);
    }

    boost::system::error_code open(implementation_type & impl, char const * device, bool is_master, boost::system::error_code & ec)
    {
        if (is_open(impl)) {
            ec = boost::asio::error::already_open;
            return ec;
        }

        boost::asio::detail::descriptor_ops::state_type state = 0;
        int fd = boost::asio::detail::descriptor_ops::open(device, O_RDWR | O_NONBLOCK | O_NOCTTY, ec);
        if (fd < 0)
            return ec;

        if (is_master) {
            if (::grantpt(fd) != 0) {
                ec = boost::asio::error::access_denied;
                return ec;
            }

            if (::unlockpt(fd) != 0) {
                ec = boost::asio::error::bad_descriptor;
                return ec;
            }
        }


        struct termios c;
        if (::tcgetattr(fd, &c) != 0) {
            ec = boost::asio::error::bad_descriptor;
            return ec;
        }
        c.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
        c.c_oflag = 0;
        c.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        if (::tcsetattr(fd, TCSAFLUSH, &c) != 0) {
            ec = boost::asio::error::bad_descriptor;
            return ec;
        }

        if (descriptor_service_.assign(impl, fd, ec)) {
            boost::system::error_code ignored_ec;
            boost::asio::detail::descriptor_ops::close(fd, state, ignored_ec);
        }

        return ec;
    }

    boost::system::error_code assign(implementation_type & impl, native_handle_type const & native_descriptor, boost::system::error_code & ec)
    {
        return descriptor_service_.assign(impl, native_descriptor, ec);
    }

    bool is_open(implementation_type const & impl) const
    {
        return descriptor_service_.is_open(impl);
    }

    boost::system::error_code close(implementation_type & impl, boost::system::error_code & ec)
    {
        return descriptor_service_.close(impl, ec);
    }

    native_handle_type native_handle(implementation_type const & impl) const
    {
        return descriptor_service_.native_handle(impl);
    }

    boost::system::error_code cancel(implementation_type & impl, boost::system::error_code & ec)
    {
        return descriptor_service_.cancel(impl, ec);
    }

    template <typename SettablePseudoterminalOption>
    boost::system::error_code set_option(implementation_type & impl, SettablePseudoterminalOption const & option, boost::system::error_code & ec)
    {
        return ec;
        //return do_set_option(impl, &pseudo_terminal_service::store_option<SettablePseudoterminalOption>, &option, ec);
    }

    template <typename GettablePseudoterminalOption>
    boost::system::error_code get_option(implementation_type const & impl, GettablePseudoterminalOption & option, boost::system::error_code & ec)
    {
        return ec;
        //return do_get_option(impl, &pseudo_terminal_service::load_option<GettablePseudoterminalOption>, &option, ec);
    }

    template <typename ConstBufferSequence>
    std::size_t write_some(implementation_type & impl, ConstBufferSequence const & buffers, boost::system::error_code & ec)
    {
        return descriptor_service_.write_some(impl, buffers, ec);
    }

    template <typename ConstBufferSequence, typename Handler>
    void async_write_some(implementation_type & impl, ConstBufferSequence const & buffers, Handler & handler)
    {
        descriptor_service_.async_write_some(impl, buffers, handler);
    }

    template <typename MutableBufferSequence>
    std::size_t read_some(implementation_type & impl, MutableBufferSequence const & buffers, boost::system::error_code & ec)
    {
        return descriptor_service_.read_some(impl, buffers, ec);
    }

    template <typename MutableBufferSequence, typename Handler>
    void async_read_some(implementation_type & impl, MutableBufferSequence const & buffers, Handler & handler)
    {
        descriptor_service_.async_read_some(impl, buffers, handler);
    }

    boost::system::error_code slave_name(implementation_type const & impl, char * device, std::size_t devicelen, boost::system::error_code & ec) const
    {
        if (::ptsname_r(descriptor_service_.native_handle(impl), device, devicelen) != 0)
            ec = boost::asio::error::bad_descriptor;
        return ec;
    }

private:
    boost::asio::detail::reactive_descriptor_service descriptor_service_;
};

extern boost::asio::io_service::id pseudo_terminal_service::id;

} }
