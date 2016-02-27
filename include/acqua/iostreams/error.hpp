#pragma once

/*!  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

namespace acqua { namespace iostreams { namespace error {

enum cryptographic_errors {
    md5_error,
    sha256_error,
    hmac_md5_error,
    hmac_sha1_error,
    hmac_sha256_error,
    hmac_sha512_error,
};

inline boost::system::error_category const & get_cryptographic_category();

boost::system::error_code make_error_code(cryptographic_errors e)
{
    return boost::system::error_code(static_cast<int>(e), get_cryptographic_category());
}

} } }


namespace boost { namespace system {

template <>
struct is_error_code_enum<acqua::iostreams::error::cryptographic_errors>
{
    static bool const value = true;
};

} }

#include <acqua/iostreams/impl/error.ipp>
