/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/utility/string_cast.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <boost/locale/encoding.hpp>
#include <iostream>
#include <memory>
#include <array>
#include <type_traits>
#include <cstring>

namespace acqua { namespace iostreams {

template <typename Char, typename CharT, typename Traits, typename Enabler = void>
class basic_istream_codecvt
{
    using streambuf_type = std::basic_streambuf<CharT, Traits>;
    using utf_traits_type =  boost::locale::utf::utf_traits<CharT>;

public:
    using char_type = Char;
    using category = boost::iostreams::source_tag;

    explicit basic_istream_codecvt(streambuf_type * sbuf)
        : sbuf_(sbuf) {}

    std::streamsize read(char_type * s, std::streamsize n)
    {
        namespace utf = boost::locale::utf;

        int ch;
        std::streamsize i = 0;
        while(i < n && (ch = static_cast<int>(sbuf_->sbumpc())) != EOF) {
            std::array<CharT, utf_traits_type::max_width> tmp;
            auto it = tmp.begin();
            for(int width = utf_traits_type::trail_length( *it++ = static_cast<CharT>(ch) ); width > 0; --width) {
                if ((ch = static_cast<int>(sbuf_->sbumpc())) == EOF)
                    return i;
                *it++ = static_cast<CharT>(ch);
            }

            it = tmp.begin();
            utf::code_point cp = utf::utf_traits<CharT>::template decode<decltype(it)>(it, tmp.end());
            if (cp != utf::illegal && cp != utf::incomplete) {
                char_type * t = utf::utf_traits<char_type>::template encode<char_type *>(cp, s);
                i += (t - s);
                s = t;
            }
        }

        return (i > 0) ? i : EOF;
    }

private:
    streambuf_type * sbuf_;
};


template <typename Char, typename CharT, typename Traits>
class basic_istream_codecvt<Char, CharT, Traits, typename std::enable_if<std::is_same<Char, CharT>::value>::type>
{
    using streambuf_type = std::basic_streambuf<CharT, Traits>;

public:
    using char_type = Char;
    using category = boost::iostreams::source_tag;

    explicit basic_istream_codecvt(streambuf_type * sbuf)
        : sbuf_(sbuf) {}

    std::streamsize read(char_type * s, std::streamsize n) const
    {
        return sbuf_->sgetn(s, n);
    }

private:
    streambuf_type * sbuf_;
};


template <typename CharT, typename Traits>
class basic_istream_locale_codecvt
{
    using streambuf_type = std::basic_streambuf<CharT, Traits>;
    using istream_type = std::basic_istream<CharT, Traits>;
    using string_type = std::basic_string<CharT, Traits>;

public:
    using char_type = char;
    using category = boost::iostreams::source_tag;

    explicit basic_istream_locale_codecvt(streambuf_type * sbuf, std::string const & charset)
        : os_(new istream_type(sbuf)), charset_(charset) {}

    std::streamsize read(char_type * s, std::streamsize n)
    {
        std::streamsize i = n;
        if (line_.size() > 1) {
            std::size_t width = std::min<std::size_t>(line_.size(), static_cast<std::size_t>(i));
            line_.erase(0, width);
            s += width;
            i -= width;
            if (i > 0) {
                line_.push_back('\n');
            }
        }

        std::basic_string<CharT, Traits> line;
        while(i > 0 && std::getline(*os_, line)) {
            trim_crln(line);
            if (!line.empty()) {
                line_ += boost::locale::conv::from_utf(line, charset_);
                std::size_t width = std::min<std::size_t>(line_.size(), static_cast<std::size_t>(i));
                std::memcpy(s, line_.c_str(), width);
                line_.erase(0, width);
                s += width;
                i -= width;
            }
            if (i > 0)
                line_.push_back('\n');
        }

        return (i == n) ? EOF : (n - i);
    }

private:
    static void trim_crln(string_type & line)
    {
        while(!line.empty()) {
            switch(line.back()) {
                case '\r':
                case '\n':
                case '\0':
                    line.pop_back();
                    break;
                default:
                    return;
            }
        }
    }

private:
    std::shared_ptr<istream_type> os_;
    std::string charset_;
    std::basic_string<char_type> line_;
};


/*!
  char <=> wchar_t を変換しながら sbuf から読み取るアダプター.
  変換する必要がない場合は、特殊化された無変換のアダプターが使われる
 */
template <typename Char, typename CharT, typename Traits>
inline basic_istream_codecvt<Char, CharT, Traits> istream_code_converter(std::basic_streambuf<CharT, Traits> * sbuf)
{
    return basic_istream_codecvt<Char, CharT, Traits>(sbuf);
}


/*!
  istream に指定する文字コードを charset として、文字コードと改行コードを変換しながら sbuf から読み取るアダプター.
 */
template <typename CharT, typename Traits>
inline basic_istream_locale_codecvt<CharT, Traits> istream_code_converter(std::basic_streambuf<CharT, Traits> * sbuf, std::string const & charset)
{
    return basic_istream_locale_codecvt<CharT, Traits>(sbuf, charset);
}

} }
