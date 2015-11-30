/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <system_error>
#include <boost/system/system_error.hpp>
#include <acqua/exception/detail/system_error_wrapper.hpp>
#include <acqua/exception/errinfo_backtrace.hpp>
#include <boost/exception/all.hpp>

namespace acqua { namespace exception {

inline void throw_error(std::error_code const & err)
{
    if (err) {
        acqua::exception::detail::system_error_wrapper<std::system_error> e(err);
        e << acqua::exception::errinfo_backtrace();
        BOOST_THROW_EXCEPTION(e);
    }
}

template <typename String>
inline void throw_error(std::error_code const & err, String const & location)
{
    if (err) {
        acqua::exception::detail::system_error_wrapper<std::system_error> e(err, location);
        e << acqua::exception::errinfo_backtrace();
        BOOST_THROW_EXCEPTION(e);
    }
}

inline void throw_error(boost::system::error_code const & err)
{
    if (err) {
        acqua::exception::detail::system_error_wrapper<boost::system::system_error> e(err);
        e << acqua::exception::errinfo_backtrace();
        BOOST_THROW_EXCEPTION(e);
    }
}

template <typename String>
inline void throw_error(boost::system::error_code const & err, String const & location)
{
    if (err) {
        acqua::exception::detail::system_error_wrapper<boost::system::system_error> e(err, location);
        e << acqua::exception::errinfo_backtrace();
        BOOST_THROW_EXCEPTION(e);
    }
}

} }
