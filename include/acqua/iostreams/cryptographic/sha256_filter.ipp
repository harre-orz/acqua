#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/cryptographic/sha256_filter.hpp>
#include <acqua/iostreams/error.hpp>

extern "C" {
#include <openssl/sha.h>
}

namespace acqua { namespace iostreams { namespace cryptographic {

class sha256_context
{
public:
    void init(boost::system::error_code & ec) noexcept
    {
        if (::SHA256_Init(&context) != 1)
            ec = make_error_code(error::sha256_error);
    }

    void update(char const * s, std::size_t n, boost::system::error_code & ec) noexcept
    {
        if (::SHA256_Update(&context, s, n) != 1)
            ec = make_error_code(error::sha256_error);
    }

    void finish(unsigned char * buffer, std::size_t, boost::system::error_code & ec) noexcept
    {
        if (::SHA256_Final(buffer, &context) != 1)
            ec = make_error_code(error::sha256_error);
    }

private:
    ::SHA256_CTX context;
};

} } }
