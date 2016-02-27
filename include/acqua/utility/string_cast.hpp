#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <string>
#include <iterator>
#include <type_traits>
#include <boost/locale.hpp>

namespace acqua { namespace utility {

template <typename It>
class convert_string
{
public:
    using char_type = typename std::char_traits<typename std::iterator_traits<It>::value_type>::char_type;

    convert_string() noexcept = default;

    convert_string(It beg, It end) noexcept
        : beg_(beg), end_(end)
    {
    }

    template <typename CharT, typename Iter, typename std::enable_if< std::is_same<CharT, char_type>::value>::type * = nullptr>
    void convert(Iter ins) const
    {
        std::copy(beg_, end_, ins);
    }

    template <typename CharT, typename Iter, typename std::enable_if<!std::is_same<CharT, char_type>::value>::type * = nullptr>
    void convert(Iter ins) const
    {
        boost::locale::utf::code_point cp;

        for(auto it = beg_; it != end_;) {
            cp = boost::locale::utf::utf_traits<char_type>::template decode<decltype(it)>(it, end_);
            if (cp != boost::locale::utf::illegal && cp != boost::locale::utf::incomplete) {
                boost::locale::utf::utf_traits<CharT>::template encode<decltype(ins)>(cp, ins);
            }
        }
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, convert_string const & rhs)
    {
        rhs.convert<Ch>(std::ostreambuf_iterator<Ch>(os));
        return os;
    }

private:
    const It beg_;
    const It end_;
};

} // utility

template <typename String, typename It, typename Allocator = typename String::allocator_type>
inline String string_cast(It beg, It end, Allocator alloc = Allocator())
{
    String res(alloc);
    utility::convert_string<It>(beg, end).template convert<typename String::value_type>(std::back_inserter(res));
    return res;
}

template <typename String, typename S,typename Allocator = typename String::allocator_type,
          typename std::enable_if<std::is_same<String, S>::value>::type * = nullptr>
inline String const & string_cast(S const & str, Allocator = Allocator())
{
    return str;
}

template <typename String, typename S, typename Allocator = typename String::allocator_type,
          typename std::enable_if<!std::is_same<String, S>::value>::type * = nullptr>
inline String string_cast(S const & str, Allocator alloc = Allocator())
{
    return string_cast<String>(str.begin(), str.end(), alloc);
}

template <typename String, typename Ch, typename Allocator = typename String::allocator_type>
inline String string_cast(Ch const * str, Allocator alloc = Allocator())
{
    return string_cast<String>(str, str + std::char_traits<Ch>::length(str), alloc);
}

template <typename It>
inline utility::convert_string<It> string_cast(It beg, It end)
{
    return utility::convert_string<It>(beg, end);
}

template <typename Ch>
inline utility::convert_string<Ch const *> string_cast(Ch const * str)
{
    return utility::convert_string<Ch const *>(str, str + std::char_traits<Ch>::length(str));
}

template <typename Ch, typename Tr = std::char_traits<Ch>, typename A = std::allocator<Ch> >
inline utility::convert_string<typename std::basic_string<Ch, Tr, A>::const_iterator> string_cast(std::basic_string<Ch, Tr, A> const & str)
{
    return utility::convert_string<typename std::basic_string<Ch, Tr, A>::const_iterator>(str.begin(), str.end());
}

}
