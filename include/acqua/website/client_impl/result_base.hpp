/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <functional>
#include <future>
#include <boost/asio/streambuf.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container/flat_map.hpp>


namespace acqua { namespace website  {

class client_result;

namespace client_impl {

template <typename Result> class socket_base;

class result_base
{
    friend socket_base<client_result>;

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
    using handler_type = std::function<void(boost::system::error_code const &, result_base &)>;
    using header_type = boost::container::flat_map<std::string, std::string>;

private:
    result_base() {}

public:
    template <typename Handler>
    explicit result_base(Handler handler)
        : handler_(handler) {}

    friend std::ostream & operator<<(std::ostream & os, result_base & rhs)
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

} } }
