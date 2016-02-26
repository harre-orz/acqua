#pragma once

/*!  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/error.hpp>

namespace acqua { namespace iostreams { namespace error {

class cryptographic_category
    : public boost::system::error_category
{
public:
    char const * name() const noexcept override
    {
        return "iostreams.cryptographic";
    }

    std::string message(int ev) const override
    {
        switch(static_cast<cryptographic_errors>(ev)) {
            case md5_error:
                return "MD5 error";
            case sha256_error:
                return "SHA256 error";
        }

        return "error";
    }
};

inline boost::system::error_category const & get_cryptographic_category()
{
    static cryptographic_category instance;
    return instance;
}

} } }
