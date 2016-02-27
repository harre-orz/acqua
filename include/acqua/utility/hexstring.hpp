#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <iostream>
#include <iomanip>
#include <type_traits>

namespace acqua { namespace utility {

template <typename It>
class hexstring
{
public:
    explicit hexstring(It beg, It end)
        : beg_(beg), end_(end)
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    }

    friend std::ostream & operator<<(std::ostream & os, hexstring const & rhs)
    {
        rhs.puts(os);
        return os;
    }

private:
    void puts(std::ostream & os) const
    {
        int hex;
        for(auto it = beg_; it != end_; ++it) {
            hex = (static_cast<std::uint8_t>(*it) >> 4);
            os << static_cast<char>(hex + (hex < 10 ? '0' : 'a' - 10));
            hex = (static_cast<std::uint8_t>(*it) & 0x0f);
            os << static_cast<char>(hex + (hex < 10 ? '0' : 'a' - 10));
        }
    }

private:
    It beg_;
    It end_;
};

}  // utility

template <typename It>
inline utility::hexstring<It> hexstring(It beg, It end)
{
    return utility::hexstring<It>(beg, end);
}

template <typename T, typename It = typename T::const_iterator>
inline utility::hexstring<It> hexstring(T const & t)
{
    return utility::hexstring<It>(t.begin(), t.end());
}

template <typename CharT, std::size_t N>
inline utility::hexstring<CharT const *> hexstring(CharT (&str)[N])
{
    return utility::hexstring<CharT const *>(str, str + N);
}

}
