#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

namespace acqua { namespace utility {

template <typename T>
struct is_1byte_type
{
    static const bool value = false;
};

template <>
struct is_1byte_type<char>
{
    static const bool value = true;
};

template <>
struct is_1byte_type<signed char>
{
    static const bool value = true;
};

template <>
struct is_1byte_type<unsigned char>
{
    static const bool value = true;
};

} }
