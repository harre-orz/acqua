/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <type_traits>
#include <iterator>
#include <acqua/utility/is_char_traits.hpp>
#include <acqua/utility/convert_string.hpp>

namespace acqua {

template <typename It>
inline acqua::utility::convert_string<It> string_cast(It beg, It end)
{
    return acqua::utility::convert_string<It>(beg, end);
}

template <
    typename CharPtr,
    typename std::enable_if<acqua::utility::is_char_traits<typename std::iterator_traits<CharPtr>::value_type>::value>::type * = nullptr  // CharPtr は 文字型のポインタであること
    >
inline acqua::utility::convert_string<typename std::iterator_traits<CharPtr>::value_type const *> string_cast(CharPtr str)
{
    using CharT = typename std::iterator_traits<CharPtr>::value_type;
    return acqua::utility::convert_string<CharT const *>(
        str, str + std::char_traits<CharT>::length(str)
    );
}

template <
    typename StringOut,
    typename Allocator = typename StringOut::allocator_type,
    typename StringIn,
    typename std::enable_if<std::is_same<StringIn, StringOut>::value>::type * = nullptr  // StringOut は StringIn と全く同じ型であること
    >
inline StringOut const & string_cast(StringIn const & str, Allocator = Allocator())
{
    return str;
}

template <
    typename StringOut,
    typename Allocator = typename StringOut::allocator_type,
    typename StringIn,
    typename std::enable_if<!std::is_same<StringIn, StringOut>::value && acqua::utility::is_char_traits<typename StringIn::value_type>::value>::type * = nullptr // StringOut は StringIn と別の型で、StringIn::value_type は 文字型であること
    >
inline StringOut string_cast(StringIn const & str, Allocator alloc = Allocator())
{
    StringOut ret(alloc);
    acqua::utility::convert_string<typename StringIn::const_iterator>(str).template convert<typename StringOut::value_type>(std::back_inserter(ret));
    return ret;
}

template <
    typename StringOut,
    typename Allocator = typename StringOut::allocator_type,
    typename CharPtr,
    typename std::enable_if<acqua::utility::is_char_traits<typename std::iterator_traits<CharPtr>::value_type>::value>::type * = nullptr  // CharPtr は文字型のポインタであること
    >
inline StringOut string_cast(CharPtr str, Allocator alloc = Allocator())
{
    using CharT = typename std::iterator_traits<CharPtr>::value_type;

    StringOut ret(alloc);
    acqua::utility::convert_string<CharT const *>(str, str + std::char_traits<CharT>::length(str)).template convert<typename StringOut::value_type>(std::back_inserter(ret));
    return ret;
}

}
