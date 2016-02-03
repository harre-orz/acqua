/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/iostreams/email_parser.hpp>

namespace acqua { namespace iostreams {

template <typename Mail>
class email_parser<Mail>::impl
{
};

template <typename Mail>
inline email_parser<Mail>::email_parser(Mail & mail)
    : error_(), impl_(new impl(error_, json)) {}

template <typename Mail>
inline std::streamsize email_parser<Mail>::write(char_type const * s, std::streamsize n)
{
    char sep[] = { '\r', '\n' };
    char_type const * end = s + n;
    if (!line_.empty() && 0 < n && *s == '\n') {
        impl_->do_
        ++s;
    }
    while(s < end) {
        char_type const * pos = std::find_of_first(s, end, sep, sep+2);
        if (pos < end) {
            line_.append(s, pos);
            impl_->do_parse_line(line_);
            line_.clear();
            s = pos + 1;
            if (s < end && *s == '\n')
                ++s;
        }
    }
    line_.append(s, end);
    return n;
}

} }
