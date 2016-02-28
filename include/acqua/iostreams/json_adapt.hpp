#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <string>

namespace acqua { namespace iostreams {

template <typename T>
struct json_adapt;

template <typename CharT>
struct json_adapt< std::basic_string<CharT> >
{

    using value_type = std::basic_string<CharT>;
    using char_type = CharT;

    json_adapt(value_type & val) : val_(val) {}

    void data(value_type & val) { val_ = val; }

private:
    value_type & val_;
};

} }
