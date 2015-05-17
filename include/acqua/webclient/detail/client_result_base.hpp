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
#include <acqua/container/sequenced_map.hpp>


namespace acqua { namespace webclient  {

class client_result;

namespace detail {

template <typename Result>
class client_socket_base;

class client_result_base
{
    friend client_socket_base<client_result>;

    struct iequals
    {
        template <typename T>
        bool operator()(T const & lhs, T const & rhs) const noexcept
        {
            return boost::algorithm::iequals(lhs, rhs);
        }
    };

public:
    using buffer_type = boost::asio::streambuf;
    using handler_type = std::function<void(boost::system::error_code const &, client_result_base &)>;
    //using header_type = acqua::container::sequenced_multimap<std::string, std::string, iequals>;
    using header_type = boost::container::flat_multimap<std::string, std::string>;

private:
    client_result_base() {}

public:
    template <typename Handler>
    explicit client_result_base(Handler handler)
        : handler_(handler) {}

    friend std::ostream & operator<<(std::ostream & os, client_result_base & rhs)
    {
        if (rhs.buffer_.size())
            os << &rhs.buffer_;
        return os;
    }

    header_type const & get_header() const
    {
        return header_;
    }

    int status_code() const
    {
        return status_code_;
    }

private:
    handler_type handler_;
    buffer_type buffer_;
    header_type header_;
    int status_code_;
};

} } }
