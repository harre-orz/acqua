/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <boost/locale.hpp>
#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace utils {

/*!
  quoted-printable 形式のテキストからデコード.
 */
template <typename CharT>
class basic_qprint_decoder
{
public:
    using char_type = CharT;

    explicit basic_qprint_decoder()
    {}

    explicit basic_qprint_decoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '=') {
                char hex[] = {0,0,0};
                if (++it == line.end()) {
                    buffer_.push_back('=');
                    break;
                }
                hex[0] = *it;
                if (!std::isxdigit(hex[0], std::locale::classic())) {
                    buffer_.push_back('=');
                    buffer_.push_back(hex[0]);
                    continue;
                }
                if (++it == line.end()) {
                    buffer_.push_back('=');
                    buffer_.push_back(hex[0]);
                    break;
                }
                hex[1] = *it;
                if (!std::isxdigit(hex[1], std::locale::classic())) {
                    buffer_.push_back('=');
                    buffer_.push_back(hex[0]);
                    buffer_.push_back(hex[1]);
                    continue;
                }
                buffer_.push_back(std::strtol(hex, nullptr, 16));
            } else {
                buffer_.push_back(*it);
            }
        }
        if (line.empty() || *line.rbegin() != '=') {
            if (charset_.empty())
                sink << acqua::string_cast(buffer_);
            else
                sink << boost::locale::conv::to_utf<char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
        if (!buffer_.empty()) {
            if (charset_.empty())
                sink << acqua::string_cast(buffer_);
            else
                sink << boost::locale::conv::to_utf<char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }
    };

private:
    std::string charset_;
    std::string buffer_;
};

using qprint_decoder = basic_qprint_decoder<char>;

} } }
