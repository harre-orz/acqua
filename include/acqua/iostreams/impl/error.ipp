/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/iostreams/error.hpp>

namespace acqua { namespace iostreams { namespace error {

namespace detail {

class json_category
    : public boost::system::error_category
{
public:
    char const * name() const noexcept override
    {
        return "iostreams.json";
    }

    std::string message(int ev) const override
    {
        switch(static_cast<enum json_errors>(ev)) {
            case illegal_state:
                return "illegal_state";
            case bad_boolean:
                return "bad_boolean";
            case bad_number:
                return "bad_number";
            case bad_string:
                return "bad_string";
            case bad_array:
                return "bad_array";
            case bad_object:
                return "bad_object";
            default:
                return "error";
        }
    }
};


} // detail

inline boost::system::error_category const & get_json_category()
{
    static detail::json_category instance;
    return instance;
}

} } }
