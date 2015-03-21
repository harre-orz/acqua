#pragma once

#include <iostream>
#include <functional>
#include <future>
#include <boost/asio/streambuf.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>

namespace acqua { namespace website {

namespace detail {

template <typename Result> class client_socket_base;

}

class client_result_future;

class client_result
{
    friend detail::client_socket_base<client_result_future>;

    struct iless
    {
        template <typename T>
        bool operator()(T const & lhs, T const & rhs) const noexcept
        {
            return boost::algorithm::lexicographical_compare(lhs, rhs);
        }
    };

public:
    using buffer_type = boost::asio::streambuf;
    using handler_type = std::function<void(boost::system::error_code const &, client_result &)>;
    using header_type = boost::container::flat_map<std::string, std::string>;

public:
    client_result() {}

    template <typename Handler>
    explicit client_result(Handler handler)
        : handler_(handler) {}

    friend std::ostream & operator<<(std::ostream & os, client_result & rhs)
    {
        if (rhs.buffer_.size())
            os << &rhs.buffer_;
        return os;
    }

private:
    handler_type handler_;
    buffer_type buffer_;
    header_type header_;
};

class client_result_future
    : public client_result
{
    friend detail::client_socket_base<client_result_future>;

public:
    using result_type = client_result;
    using buffer_type = typename result_type::buffer_type;
    using handler_type = typename result_type::handler_type;

public:
    client_result_future()
        : result_type(&client_result_future::callback)
        , future_(promise_.get_future()) {}

    boost::system::error_code const & error() const
    {
        future_.wait();
        return error_;
    }

    friend std::ostream & operator<<(std::ostream & os, client_result_future & rhs)
    {
        rhs.future_.wait();
        return os << static_cast<result_type &>(rhs);
    }

private:
    static void callback(boost::system::error_code const & error, result_type & res)
    {
        static_cast<client_result_future &>(res).error_ = error;
        static_cast<client_result_future &>(res).promise_.set_value();
    }

private:
    boost::system::error_code error_;
    std::promise<void> promise_;
    mutable std::future<void> future_;
};

} }
