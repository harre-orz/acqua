/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>

namespace acqua { namespace email { namespace error {

enum conversion_errors {
    not_address,
    parse_aborted,
    invalid_charset,
};

inline boost::system::error_category const & get_conversion_category();

boost::system::error_code make_error_code(conversion_errors e)
{
    return boost::system::error_code(static_cast<int>(e), get_conversion_category());
}

} } }

namespace boost { namespace system {

template <> struct is_error_code_enum<acqua::email::error::conversion_errors>
{
    static bool const value = true;
};

} }

#include <acqua/email/impl/error.ipp>
