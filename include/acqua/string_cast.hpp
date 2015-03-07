#pragma once

#include <type_traits>
#include <iterator>
#include <string>
#include <acqua/utility/is_char_traits.hpp>
#include <acqua/utility/convert_string.hpp>

namespace acqua {

template <typename It>
inline acqua::utility::convert_string<It> string_cast(It beg, It end)
{
    return acqua::utility::convert_string<It>(beg, end);
}

template <typename CharT, std::size_t N>
inline acqua::utility::convert_string<CharT const *> string_cast(CharT const (&str)[N])
{
    return acqua::utility::convert_string<CharT const *>(
        str, str + N - 1
    );
}

template <typename CharPtr, typename std::enable_if<acqua::utility::is_char_traits<typename std::iterator_traits<CharPtr>::value_type>::value>::type * = nullptr>
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
    typename std::enable_if<std::is_same<StringIn, StringOut>::value>::type * = nullptr
    >
inline StringOut const & string_cast(StringIn const & str, Allocator = Allocator())
{
    return str;
}

template <
    typename StringOut,
    typename Allocator = typename StringOut::allocator_type,
    typename StringIn,
    typename std::enable_if<!std::is_same<StringIn, StringOut>::value>::type * = nullptr
    >
inline StringOut string_cast(StringIn const & str, Allocator alloc = Allocator())
{
    StringOut ret(alloc);
    acqua::utility::convert_string<typename StringIn::const_iterator>(str).template convert< typename StringOut::value_type>(std::back_inserter(ret));
    return ret;
}

}
