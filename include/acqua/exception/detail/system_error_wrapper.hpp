/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <exception>
#include <boost/exception/exception.hpp>

namespace acqua { namespace exception { namespace detail {

//! boost::system_error か std::system_error に boost::exception を付加したラッパークラス
template <typename SystemError>
class system_error_wrapper
    : public SystemError
    , public boost::exception
{
public:
    template <typename ErrorCode>
    system_error_wrapper(ErrorCode err)
        : SystemError(err)
    {
    }

    template <typename ErrorCode, typename String>
    system_error_wrapper(ErrorCode err, String const & location)
        : SystemError(err, location)
    {
    }
};

} } }
