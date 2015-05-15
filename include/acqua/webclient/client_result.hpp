/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <future>
#include <boost/system/error_code.hpp>
#include <acqua/webclient/detail/client_result_base.hpp>


namespace acqua { namespace webclient {

class client_result
    : public detail::client_result_base
{
public:
    using result = detail::client_result_base;
    using buffer_type = typename result::buffer_type;
    using handler_type = typename result::handler_type;

    client_result()
        : result(&client_result::callback)
        , future_(promise_.get_future()) {}

    boost::system::error_code const & error() const
    {
        future_.wait();
        return error_;
    }

    friend std::ostream & operator<<(std::ostream & os, client_result & rhs)
    {
        rhs.future_.wait();
        return os << static_cast<result &>(rhs);
    }

private:
    static void callback(boost::system::error_code const & error, result & res)
    {
        static_cast<client_result &>(res).error_ = error;
        static_cast<client_result &>(res).promise_.set_value();
    }

    boost::system::error_code error_;
    std::promise<void> promise_;
    mutable std::future<void> future_;
};

} }
