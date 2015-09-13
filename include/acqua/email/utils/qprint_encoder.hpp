/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <boost/locale.hpp>

namespace acqua { namespace email { namespace utils {

/*!
  quoted-printable 形式のテキストにエンコード.
 */
template <typename CharT>
class basic_qprint_encoder
{
public:
    using char_type = CharT;

    explicit basic_qprint_encoder()
    {}

    explicit basic_qprint_encoder(std::string const & charset, std::size_t line_break = 76)
        : charset_(charset), line_break_(line_break)
    {
        if (line_break_ == 0)
            line_break_ = std::numeric_limits<decltype(line_break_)>::max();
    }

    template <typename Source>
    void read(Source & src, std::string & line)
    {
        std::basic_string<CharT> tmp;
        if (!std::getline(src, tmp))
            return;

        for(char ch : charset_.empty() ? acqua::string_cast<std::string>(tmp) : boost::locale::conv::from_utf<char>(tmp, charset_)) {
            if (line.size() >= line_break_)
                line += "=\r\n";
            if (ch == '=')                  put(line, '=');
            else if (33 <= ch && ch <= 126) line += ch;
            else                            put(line, ch);
        }
        line += "\r\n";
    }

private:
    void put(std::string & line, unsigned char hex)
    {
        line += '=';
        char upper = (hex >> 4) & 0x0f;
        char lower = hex        & 0x0f;
        line += (upper < 10) ? upper + '0' : upper + 'A' - 10;
        line += (lower < 10) ? lower + '0' : lower + 'A' - 10;
    }

private:
    std::string charset_;
    std::size_t line_break_;
};

using qprint_encoder = basic_qprint_encoder<char>;
using qprint_wencoder = basic_qprint_encoder<wchar_t>;

} } }
