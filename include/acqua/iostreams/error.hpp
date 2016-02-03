/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>

namespace acqua { namespace iostreams { namespace error {

enum json_errors {
    illegal_state,
    bad_boolean,
    bad_number,
    bad_string,
    bad_array,
    bad_object,
};

inline boost::system::error_category const & get_json_category();

boost::system::error_code make_error_code(json_errors e)
{
    return boost::system::error_code(static_cast<int>(e), get_json_category());
}

} } }

namespace boost { namespace system {

template <> struct is_error_code_enum<acqua::iostreams::error::json_errors>
{
    static bool const value = true;
};

} }

#include <acqua/iostreams/impl/error.ipp>
