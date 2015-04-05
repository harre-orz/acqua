/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <type_traits>
#include <boost/locale.hpp>

#include <acqua/utility/is_charactor.hpp>

namespace acqua { namespace utility {

template <typename String, typename Enabler = void>
class convert_string;


template <typename It>
class convert_string<It, typename std::enable_if<acqua::utility::is_charactor<typename std::iterator_traits<It>::value_type>::value>::type>
{
public:
    using char_type = typename std::iterator_traits<It>::value_type;

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

} }
