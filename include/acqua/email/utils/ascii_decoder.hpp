/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <algorithm>
#include <bitset>
#include <boost/locale.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <acqua/string_cast.hpp>
#include <acqua/email/utils/ascii_traits.hpp>

namespace acqua { namespace email { namespace utils {

/*!
  7bit もしくは 8bit のテキストデコーダ.
 */
template <typename CharT>
class basic_ascii_decoder
    : private ascii_traits
{
public:
    using char_type = CharT;

    explicit basic_ascii_decoder(std::string const & charset, bool is_format_flowed, bool is_delete_space)
        : ascii_traits(is_format_flowed, is_delete_space), charset_(charset)
    {
    }

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        bool newline = true;
        if (ascii_traits::is_format_flowed() && line.size() > 1 && std::isspace(*line.rbegin(), std::locale::classic()))
            newline = (line.size() >= 3 && (*line.rbegin()-1) == '-' && (*line.rbegin()-2) == '-');
        auto temp = boost::locale::conv::to_utf<char_type>(line, charset_);
        if (ascii_traits::is_delete_space())
            boost::trim_right(temp);
        sink << acqua::string_cast(temp);
        if (newline)
            sink << std::endl;
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }

private:
    std::string charset_;
};

using ascii_decoder = basic_ascii_decoder<char>;

} } }
