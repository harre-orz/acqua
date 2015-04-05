/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <type_traits>
#include <iterator>
#include <acqua/utility/convert_string.hpp>

namespace acqua {

template <typename String, typename S, typename Allocator = typename String::allocator_type, typename std::enable_if<std::is_same<String, S>::value>::type * = nullptr>
inline static String const & string_cast(S const & str, Allocator const & = Allocator())
{
    return str;
}

template <typename String, typename Iterator, typename Allocator = typename String::allocator_type>
inline static String string_cast(Iterator beg, Iterator end, Allocator const & alloc = Allocator())
{
    String res(alloc);
    utility::convert_string<Iterator>(beg, end).template convert<typename String::value_type>(std::back_inserter(res));
    return res;
}

template <typename String, typename S, typename Allocator = typename String::allocator_type, typename std::enable_if<!std::is_same<String, S>::value>::type * = nullptr>
inline static String string_cast(S const & str, Allocator const & alloc = Allocator())
{
    return string_cast<String>(str.begin(), str.end(), alloc);
}

template <typename String, typename Ch, typename Allocator = typename String::allocator_type>
inline static String string_cast(Ch const * const str, Allocator const & alloc = Allocator())
{
    return string_cast<String>(str, str + std::char_traits<Ch>::length(str), alloc);
}

template <typename Ch>
inline static utility::convert_string<Ch const *> string_cast(Ch const * str)
{
    return utility::convert_string<Ch const *>(str, str + std::char_traits<Ch>::length(str));
}

template <typename Ch, typename Tr = std::char_traits<Ch>, typename A = std::allocator<Ch> >
inline static utility::convert_string<typename std::basic_string<Ch, Tr, A>::const_iterator> string_cast(std::basic_string<Ch, Tr, A> const & str)
{
    return utility::convert_string<typename std::basic_string<Ch, Tr, A>::const_iterator>(str.begin(), str.end());
}

template <typename It>
inline static utility::convert_string<It> string_cast(It beg, It end)
{
    return utility::convert_string<It>(beg, end);
}

}
