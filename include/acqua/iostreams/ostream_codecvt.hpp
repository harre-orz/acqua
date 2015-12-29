#pragma once

#include <memory>
#include <iostream>
#include <boost/iostreams/categories.hpp>
#include <boost/locale.hpp>

namespace acqua { namespace iostreams { namespace detail {

template <typename Char, typename CharT, typename Traits, typename Enabler = void>
class basic_ostream_codecvt
{
public:
    using char_type = Char;
    using category = boost::iostreams::sink_tag;

    explicit basic_ostream_codecvt(std::basic_streambuf<CharT, Traits> * sbuf)
        : sbuf_(sbuf) {}

    std::streamsize write(char_type const * s, std::streamsize n) const
    {
        auto str = boost::locale::conv::utf_to_utf<CharT>(s, s + n);
        sbuf_->sputn(str.c_str(), static_cast<std::streamsize>(str.size()));
        return n;
    }

private:
    std::basic_streambuf<CharT, Traits> * sbuf_;
};


template <typename Char, typename CharT, typename Traits>
class basic_ostream_codecvt<Char, CharT, Traits, typename std::enable_if<std::is_same<Char, CharT>::value>::type>
{
public:
    using char_type = Char;
    using category = boost::iostreams::sink_tag;

    explicit basic_ostream_codecvt(std::basic_streambuf<CharT, Traits> * sbuf)
        : sbuf_(sbuf) {}

    std::streamsize write(char_type const * s, std::streamsize n) const
    {
        return sbuf_->sputn(s, n);
    }

private:
    std::basic_streambuf<CharT, Traits> * sbuf_;
};


template <typename Char, typename CharT, typename Traits>
class basic_ostream_locale_codecvt
{
public:
    using char_type = Char;
    struct category : boost::iostreams::sink_tag, boost::iostreams::closable_tag {};

    explicit basic_ostream_locale_codecvt(std::basic_streambuf<CharT, Traits> * sbuf, std::string const & charset)
        : os_(new std::basic_ostream<CharT, Traits>(sbuf)), charset_(charset) {}

    std::streamsize write(char_type const * beg, std::streamsize size)
    {
        if (size <= 0)
            return size;

        char_type sep[] = { '\r', '\n' };
        char_type const * end = beg + size;

        // 前回の最後が '\r' のときで 今回の最初が '\n' のときは '\n' を飛ばす
        if (last_ == '\r' && *beg == '\n')
            ++beg;
        last_ = end[-1];

        for(char_type const * it; (it = std::find_first_of(beg, end, sep, sep+2)) != end; beg = it) {
            line_.append(beg, it);  // 改行コードは含まない
            *os_ << boost::locale::conv::to_utf<CharT>(line_, charset_) << std::endl;
            line_.clear();
            if (*it == '\r')
                ++it;
            if (it == end)
                return size;
            if (*it == '\n')
                ++it;
            if (it == end)
                return size;
        }

        line_.append(beg, end);
        return size;
    }

    void close()
    {
        if (!line_.empty()) {
            *os_ << boost::locale::conv::to_utf<CharT>(line_, charset_);
            line_.clear();
        }
    }

private:
    std::shared_ptr< std::basic_ostream<CharT, Traits> > os_;
    std::string charset_;
    std::basic_string<char_type> line_ = {};
    char_type last_ = '\0';
};

}  // detail


/*!
  char <=> wchar_t を変換しながら sbuf に書き込むアダプター。
  変換する必要がない場合は、特殊化された無変換のアダプターが使われる
 */

template <typename Char, typename CharT, typename Traits>
detail::basic_ostream_codecvt<Char, CharT, Traits> ostream_code_converter(std::basic_streambuf<CharT, Traits> * sbuf)
{
    return detail::basic_ostream_codecvt<Char, CharT, Traits>(sbuf);
}


/*!
  文字コード・改行コードを変換しながら sbuf に書き込むアダプター
 */
template <typename Char, typename CharT, typename Traits>
detail::basic_ostream_locale_codecvt<Char, CharT, Traits> ostream_code_converter(std::basic_streambuf<CharT, Traits> * sbuf, std::string const & charset)
{
    return detail::basic_ostream_locale_codecvt<Char, CharT, Traits>(sbuf, charset);
}

} }
