/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <acqua/asio/read_until.hpp>


namespace acqua { namespace webclient { namespace detail {

template <typename Result>
class client_socket_base
    : public std::ostream
{
public:
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using buffer_type = typename Result::buffer_type;
    using result_type = typename Result::result;

public:
    client_socket_base()
        : std::ostream(&buffer_) {}

    virtual ~client_socket_base() {}
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

    void set_gzip()
    {
        in_.push(boost::iostreams::gzip_decompressor());
    }

    void set_zlib()
    {
        in_.push(boost::iostreams::zlib_decompressor());
    }

    void buffer_copy(std::size_t size)
    {
        std::streamsize rest;
        char buf[4096];
        std::istream is(&buffer_);
        std::ostream os(&result_->buffer_);
        in_.push(is, size);

        while(size > 0) {
            rest = boost::iostreams::read(in_, buf, std::min(size, sizeof(buf)));
            os.write(buf, rest);
            size -= rest;
        }

        in_.reset();
    }

    typename result_type::header_type & get_header()
    {
        return result_->header_;
    }

    buffer_type & temp_buffer()
    {
        return result_->buffer_;
    }

    int & status_code()
    {
        return result_->status_code_;
    }

    template <typename Socket>
    void move_start(Socket & socket)
    {
        socket.result_ = result_;
    }

protected:
    endpoint_type endpoint_;
    boost::shared_ptr<result_type> result_;
    buffer_type buffer_;
    boost::iostreams::filtering_streambuf<boost::iostreams::input> in_;
    bool is_keep_alive_ = false;
    bool is_callback_;
};

} } }
