#pragma once

#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <acqua/asio/read_until.hpp>
#include <acqua/website/client_impl/socket_base.hpp>


namespace acqua { namespace website { namespace client_impl {

template <typename Result>
class socket_base
    : public std::ostream
{
public:
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using buffer_type = typename Result::buffer_type;
    using result_type = typename Result::result;

public:
    socket_base()
        : std::ostream(&buffer_) {}

    virtual ~socket_base() {}
    virtual void cancel() = 0;

    endpoint_type const & endpoint() const noexcept
    {
        return endpoint_;
    }

    bool is_keep_alive() const noexcept
    {
        return is_keep_alive_;
    }

    boost::shared_ptr<Result> start()
    {
        decltype(result_) expected;
        decltype(result_) desired(new Result());
        if (!boost::atomic_compare_exchange(&result_, &expected, desired)) {
            return nullptr;
        }

        async_write();
        return boost::static_pointer_cast<Result>(result_);
    }

    template <typename Handler>
    void async_start(Handler handler)
    {
        decltype(result_) expected;
        decltype(result_) desired(new result_type(handler));
        if (!boost::atomic_compare_exchange(&result_, &expected, desired)) {
            handler(boost::system::error_code(), *desired);
            return;
        }

        async_write();
    }

protected:
    virtual void async_write() = 0;

    void callback(boost::system::error_code const & error, boost::asio::io_service & io_service)
    {
        if (auto result = boost::atomic_exchange(&result_, decltype(result_)())) {
            io_service.post([error, result]() {
                    result->handler_(error, *result);
                });
        }
    }

    void buffer_copy(std::size_t size)
    {
        // TODO: content-encoding による圧縮形式に対応
        result_->buffer_.sputn(boost::asio::buffer_cast<char const *>(buffer_.data()), size);
        buffer_.consume(size);
    }

    typename result_type::header_type & get_header()
    {
        return result_->header_;
    }

protected:
    endpoint_type endpoint_;
    boost::shared_ptr<result_type> result_;
    buffer_type buffer_;
    bool is_keep_alive_ = false;
};

} } }
