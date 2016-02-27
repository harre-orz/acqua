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
#include <boost/lexical_cast.hpp>
#include <boost/io/ios_state.hpp>

namespace acqua { namespace utility {

template <typename It>
class hexstring
{
public:
    explicit hexstring(It beg, It end)
        : beg_(beg), end_(end) {}

    std::string string() const
    {
        return boost::lexical_cast<std::string>(*this);
    }

    friend std::ostream & operator<<(std::ostream & os, hexstring const & rhs)
    {
        rhs.puts(os);
        return os;
    }

private:
    void puts(std::ostream & os) const
    {
        boost::io::ios_flags_saver ifs(os);
        os << std::hex;
        for(auto it = beg_; it != end_; ++it)
            os << std::setfill('0') << std::setw(2) << static_cast<uint>(*it);
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
