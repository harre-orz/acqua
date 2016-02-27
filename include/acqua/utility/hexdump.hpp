#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/size_t.hpp>

namespace acqua { namespace utility {

template <typename It, bool C = true, std::size_t L = 16, typename Enabler = void>
class hexdump
{
    using size0_type = typename boost::mpl::size_t<0>::type;

public:
    using canonical_type = typename boost::mpl::bool_<C>::type;
    using length_type = typename boost::mpl::size_t<L>::type;

    explicit hexdump(It beg, It end)
        : beg_(beg), end_(end)
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    }

    friend std::ostream & operator<<(std::ostream & os, hexdump const & rhs)
    {
        rhs.puts(os);
        return os;
    }

private:
    void puts(std::ostream & os) const
    {
        auto carry = static_cast<int>(std::log10(std::distance(beg_, end_)) + 1);

        auto it = beg_;
        put_linum(os, 0, carry);
        if (it != end_) {
            put_hex(os, *it++);
            while(it != end_) {
                if (!put_endline(os, it, carry, length_type()))
                    os << ' ';
                put_hex(os, *it++);
            }
        }

        put_canon(os, std::distance(beg_, end_), end_, canonical_type());
        os << std::endl;
    }

    template <typename SizeT>
    bool put_endline(std::ostream & os, It it, int carry, SizeT) const
    {
        auto pos = std::distance(beg_, it);
        if ((pos % static_cast<std::ptrdiff_t>(SizeT::value)) != 0)
            return false;

        put_canon(os, pos, it, canonical_type());
        os << std::endl;
        put_linum(os, pos, carry);
        return true;
    }

    bool put_endline(std::ostream &, std::ptrdiff_t, It, int, size0_type) const
    {
        return false;
    }

    void put_canon(std::ostream & os, std::ptrdiff_t pos, It it, boost::mpl::true_) const
    {
        pos %= length_type::value;
        if (pos > 0)
            os << std::setfill(' ') << std::setw(static_cast<int>((static_cast<std::ptrdiff_t>(length_type::value) - pos) * 3)) << ' ';
        else
            pos = length_type::value;

        std::advance(it, -pos);
        os << ' ' << '"';
        for(;--pos; ++it)
            os << (std::isprint(*it) ? *it : '.');
        os << '"';
    }

    void put_canon(std::ostream &, std::ptrdiff_t, It, boost::mpl::false_) const
    {
    }

    template <typename CharT>
    void put_hex(std::ostream & os, CharT ch) const
    {
        int hex;
        hex = (static_cast<std::uint8_t>(ch) >> 4);
        os << static_cast<char>(hex + (hex < 10 ? '0' : 'a' - 10));
        hex = (static_cast<std::uint8_t>(ch) & 0x0f);
        os << static_cast<char>(hex + (hex < 10 ? '0' : 'a' - 10));
    }

    void put_linum(std::ostream & os, std::ptrdiff_t pos, int carry) const
    {
        os << std::setfill('0') << std::setw(carry+1) << pos << ' ' << '|' << ' ';
    }

private:
    It beg_;
    It end_;
};

}  // utility

template <bool Canonical = true, std::size_t Length = 16, typename It>
inline utility::hexdump<It, Canonical, Length> hexdump(It beg, It end)
{
    return utility::hexdump<It, Canonical, Length>(beg, end);
}

template <bool Canonical = true, std::size_t Length = 16, typename T, typename It = typename T::const_iterator>
inline utility::hexdump<It, Canonical, Length> hexdump(T const & t)
{
    return utility::hexdump<It, Canonical, Length>(t.begin(), t.end());
}

template <bool Canonical = true, std::size_t Length = 16>
inline utility::hexdump<char const *, Canonical, Length> hexdump(char const * str)
{
    return utility::hexdump<char const *, Canonical, Length>(str, str + std::char_traits<char>::length(str));
}

}
