/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <boost/locale.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <acqua/email/utils/ascii_traits.hpp>

namespace acqua { namespace email { namespace utils {

/*!
  7bit もしくは 8bit のテキストエンコーダ.
*/
template <typename CharT>
class basic_ascii_encoder
    : private ascii_traits
{
public:
    using char_type = CharT;

    explicit basic_ascii_encoder(std::string const & charset, bool is_format_flowed, bool is_delete_space, std::size_t line_break = 76)
        : ascii_traits(is_format_flowed, is_delete_space), charset_(charset), line_break_(line_break)
    {
        if (line_break_ == 0) line_break_ = std::numeric_limits<decltype(line_break_)>::max();
    }

    template <typename Source>
    void read(Source & src, std::string & line)
    {
        std::basic_string<CharT> tmp;
        if (!std::getline(src, tmp))
            return;

        if (ascii_traits::is_format_flowed() && ascii_traits::is_delete_space()) {
            // 文字数が多い場合は改行する
            std::string buf = boost::locale::conv::from_utf<char>(tmp, charset_);
            // FIXME: 可変長文字の場合、文脈に関係なく改行コードを入れているので、タコなメーラだと文字化けする恐れがある
            while(buf.size() > line_break_) {
                line += buf.substr(0, line_break_);
                line += " \r\n";
                buf.erase(0, line_break_);
            }
            line += buf;
        } else {
            line = boost::locale::conv::from_utf<char>(tmp, charset_);
        }
        line += "\r\n";
    }

private:
    std::string charset_;
    std::size_t line_break_;
};

using ascii_encoder = basic_ascii_encoder<char>;
using ascii_wencoder = basic_ascii_encoder<wchar_t>;

} } }
