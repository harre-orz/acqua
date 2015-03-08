/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace utility {

/*!
  文字型であるかを判定する
 */
template <typename T>
struct is_char_traits
{
    static const bool value = false;
};

template <>
struct is_char_traits<char>
{
    static const bool value = true;
};

template <>
struct is_char_traits<wchar_t>
{
    static const bool value = true;
};

} }
