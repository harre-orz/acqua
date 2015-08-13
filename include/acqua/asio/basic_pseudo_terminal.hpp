#pragma once

#include <boost/asio/basic_io_object.hpp>
#include <acqua/asio/pseudo_terminal_service.hpp>

namespace acqua { namespace asio {

template <typename PseudoTerminalService>
class basic_pseudo_terminal
    : public boost::asio::basic_io_object<PseudoTerminalService>
{
public:
    using native_handle_type = typename PseudoTerminalService::native_handle_type;
    using lowest_layer_type = basic_pseudo_terminal<PseudoTerminalService>;

public:
    explicit basic_pseudo_terminal(boost::asio::io_service & io_service)
        : boost::asio::basic_io_object<PseudoTerminalService>(io_service)
    {
    }

    explicit basic_pseudo_terminal(boost::asio::io_service & io_service, native_handle_type const & native_pseudo_terminal)
        : boost::asio::basic_io_object<PseudoTerminalService>(io_service)
    {
        boost::system::error_code ec;
        this->get_service().assign(this->get_implementation(), native_pseudo_terminal, ec);
        boost::asio::detail::throw_error(ec, "assign");
    }

protected:
    ~basic_pseudo_terminal() = default;

public:
    lowest_layer_type & lowest_layer()
    {
        return *this;
    }

    lowest_layer_type const & lowest_layer() const
    {
        return *this;
    }

    native_handle_type native_handle() const
    {
        return this->get_service().native_handle(this->get_implementation());
    }

    bool is_open() const
    {
        return this->get_service().is_open(this->get_implementation());
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

    template <typename ConstBufferSequence>
    std::size_t write_some(ConstBufferSequence const & buffers)
    {
        boost::system::error_code ec;
        std::size_t size = this->get_service().write_some(this->get_implementation(), buffers, ec);
        boost::asio::detail::throw_error(ec, "write_some");
        return size;
    }

    template <typename ConstBufferSequence>
    std::size_t write_some(ConstBufferSequence const & buffers, boost::system::error_code & ec)
    {
        return this->get_service().write_some(this->get_implementation(), buffers, ec);
    }

    template <typename ConstBufferSequence, typename Handler>
    void async_write_some(ConstBufferSequence const & buffers, Handler handler)
    {
        this->get_service().async_write_some(this->get_implementation(), buffers, handler);
    }

    template <typename MutableBufferSequence>
    std::size_t read_some(MutableBufferSequence const & buffers)
    {
        boost::system::error_code ec;
        std::size_t size = this->get_service().read_some(this->get_implementation(), buffers, ec);
        boost::asio::detail::throw_error(ec, "read_some");
        return size;
    }

    template <typename MutableBufferSequence>
    std::size_t read_some(MutableBufferSequence const & buffers, boost::system::error_code & ec)
    {
        return this->get_service().read_some(this->get_implementation(), buffers, ec);
    }

    template <typename MutableBufferSequence, typename Handler>
    void async_read_some(MutableBufferSequence const & buffers, Handler handler)
    {
        this->get_service().async_read_some(this->get_implementation(), buffers, handler);
    }
};


template <typename PseudoTerminalService>
class basic_pseudo_terminal_slave;


template <typename PseudoTerminalService>
class basic_pseudo_terminal_master
    : public basic_pseudo_terminal<PseudoTerminalService>
{
    friend basic_pseudo_terminal_slave<PseudoTerminalService>;

public:
    enum open_mode { with_open };

    using native_handle_type = typename PseudoTerminalService::native_handle_type;
    using lowest_layer_type = basic_pseudo_terminal_master<PseudoTerminalService>;

    explicit basic_pseudo_terminal_master(boost::asio::io_service & io_service)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service)
    {
    }

    explicit basic_pseudo_terminal_master(boost::asio::io_service & io_service, native_handle_type handle)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service, handle)
    {
    }

    explicit basic_pseudo_terminal_master(boost::asio::io_service & io_service, open_mode)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service)
    {
        open();
    }

    void open()
    {
        boost::system::error_code ec;
        this->get_service().open(this->get_implementation(), "/dev/ptmx", true, ec);
        boost::asio::detail::throw_error(ec, "open");
    }

    boost::system::error_code open(boost::system::error_code & ec)
    {
        return this->get_service().open(this->get_implementation(), "/dev/ptmx", true, ec);
    }

    std::string slave_name() const
    {
        char device[256];
        boost::system::error_code ec;
        this->get_service().slave_name(this->get_implementation(), device, sizeof(device), ec);
        boost::asio::detail::throw_error(ec, "ptsname");
        return std::string(device);
    }

    std::string slave_name(boost::system::error_code & ec) const
    {
        char device[256];
        if (this->get_service().slave_name(this->get_implementation(), device, sizeof(device), ec))
            device[0] = '\0';
        return std::string(device);
    }
};



template <typename PseudoTerminalService>
class basic_pseudo_terminal_slave
    : public basic_pseudo_terminal<PseudoTerminalService>
{
public:
    using native_handle_type = typename PseudoTerminalService::native_handle_type;
    using lowest_layer_type = basic_pseudo_terminal_slave<PseudoTerminalService>;
    using master_type = basic_pseudo_terminal_master<PseudoTerminalService>;

    explicit basic_pseudo_terminal_slave(boost::asio::io_service & io_service)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service)
    {
    }

    explicit basic_pseudo_terminal_slave(boost::asio::io_service & io_service, native_handle_type handle)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service, handle)
    {
    }

    explicit basic_pseudo_terminal_slave(boost::asio::io_service & io_service, master_type const & master)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service)
    {
        open(master);
    }

    explicit basic_pseudo_terminal_slave(boost::asio::io_service & io_service, std::string const & device)
        : basic_pseudo_terminal<PseudoTerminalService>(io_service)
    {
        open(device);
    }

    void open(master_type const & master)
    {
        char device[256];
        boost::system::error_code ec;
        master.get_service().slave_name(master.get_implementation(), device, sizeof(device), ec);
        boost::asio::detail::throw_error(ec, "ptsname");
        this->get_service().open(this->get_implementation(), device, false, ec);
        boost::asio::detail::throw_error(ec, "open");
    }

    boost::system::error_code open(master_type const & master, boost::system::error_code & ec)
    {
        char device[256];
        if (!master.get_service().slave_name(master.get_implementation(). device, sizeof(device), ec))
            return this->get_service().open(this->get_implementation(), device, false, ec);
        return ec;
    }

    void open(std::string const & device)
    {
        boost::system::error_code ec;
        this->get_service().open(this->get_implementation(), device.c_str(), false, ec);
        boost::asio::detail::throw_error(ec, "open");
    }

    boost::system::error_code open(std::string const & device, boost::system::error_code & ec)
    {
        return this->get_service().open(this->get_implementation(), device.c_str(), false, ec);
    }

};

} }
