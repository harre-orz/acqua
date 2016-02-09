/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/email/encode_mimeheader.hpp>
#include <acqua/email/error.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/string_cast.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/locale/encoding.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace acqua { namespace email { namespace detail {

template <typename CharT>
class encode_mimeheader_impl
{
public:
    using string_type = std::basic_string<CharT>;
    using utf_traits_type = boost::locale::utf::utf_traits<CharT>;

private:
    using string_iter = typename string_type::const_iterator;

public:
    explicit encode_mimeheader_impl(std::ostream & os, std::string const & charset)
        : os_(os), charset_(charset) {}

    encode_mimeheader_impl & operator()(string_type const & key, string_type const & val)
    {
        if (!is_name(key))
            BOOST_THROW_EXCEPTION( syntax_error() );

        // ヘッダー名を書き込む
        os_ << acqua::string_cast(key) << ':' << ' ';
        cnt_ += key.size()+2;

        // ヘッダー値を書き込む
        if (val.empty())
            ; // 何もしない
        else if (is_ascii(val) && !std::isspace(val.front()) && !std::isspace(val.back()))
            plain_encode(val);  // val がアスキー文字でかつ両端に空白を含まない場合は、そのまま出力できる
        else
            rfc2047_encode(val);
        return *this;
    }

    template <typename Params>
    encode_mimeheader_impl & operator()(Params const & params)
    {
        for(auto const & kv : params) {
            if (!is_name(kv.first))
                BOOST_THROW_EXCEPTION( syntax_error() );

            // 改行調整
            if (cnt_ - 2 < max_) {
                os_ << ';' << ' ';
                cnt_ += 2;
            } else {
                os_ << ';' << '\r' << '\n' << '\t';
                cnt_ = 1;
            }

            // disp名、disp値を書き込む
            if (kv.second.empty()) {
                os_ << acqua::string_cast(kv.first);
                cnt_ += kv.first.size();
            } else if (is_ascii(kv.second)) {
                os_ << acqua::string_cast(kv.first) << '=' << '"';
                plain_encode(kv.second);
                os_ << '"';
                cnt_ += kv.first.size() + 3;
            } else {
                rfc2231_encode(kv.first, kv.second);
            }
        }

        return *this;
    }

private:
    bool is_name(string_type const & str) const
    {
        return (!str.empty() && std::find_if(str.begin(), str.end(), [this](CharT ch) { return !(std::isalnum(ch, loc) || ch == '-'); }) == str.end());
    }

    bool is_ascii(string_type const & str) const
    {
        return (std::find_if(str.begin(), str.end(), [this](CharT ch) { return !std::isprint(ch, loc); }) == str.end());
    }

    // cnt_ が max_ を超えないようにしつつ str に含まれる単語単位で改行に区切る
    void plain_encode(string_type const & str)
    {
        auto a = str.begin();
        auto end = str.end();

        //　区切りを探す
        auto b = std::find_if(a, end, [this](CharT ch) { return std::isspace(ch, loc); });
        while(b != end) {
            // 区切りでない文字を探す
            auto c = std::find_if(b, end, [this](CharT ch) { return !isspace(ch, loc); });
            if (c == end) {
                // 文字がないときは、空白文字も書き出して終わり
                b = c;
                break;
            }

            // さらに次の空白もしくは文字の終端を探す
            auto d = std::find_if(c, end, [this](CharT ch) { return std::isspace(ch, loc); });
            if (cnt_ + (d - a) < max_) {
                os_.write(&*a, c - a);
                cnt_ += (c - a);
            } else {
                os_.write(&*a, b - a);
                os_ << '\r' << '\n';
                os_.write(&*b, c - b);
                cnt_ += (c - b);
            }
            a = c;
            b = d;
        }

        os_.write(&*a, b - a);
        cnt_ += (b - a);
    }

    void rfc2047_encode(string_type const & str)
    {
        auto a = str.begin();
        auto end = str.end();
        for(auto b = a; b != end;) {
            for(int width = utf_traits_type::trail_length(*b++); width > 0; width = 0)
                b += width;

            // とりあえず max_ に2倍のバッファ持たさせてくお
            if ((b - a) > max_ / 2) {
                os_ << '=' << '?' << acqua::string_cast(charset_) << '?' << 'B' << '?'
                      << base64_encode(a, b, charset_) << '?' << '=' << '\r' << '\n'
                      << '\t';
                a = b;
            }
        }
        if (a != end) {
            os_ << '=' << '?' << acqua::string_cast(charset_) << '?' << 'B' << '?'
                  << base64_encode(a, end, charset_) << '?' << '=';
        }
    }

    void rfc2231_encode(string_type const & key, string_type const & val)
    {
        int idx = 0;
        auto a = val.begin();
        auto end = val.end();
        for(auto b = a; b != end;) {
            for(int width = utf_traits_type::trail_length(*b++); width > 0; width = 0)
                b += width;

            // とりあえず max_ に2倍のバッファ持たさせておく
            if ((b - a) > max_ / 2) {
                os_ << key << '*'  << ++idx << '*' << '=';
                if (idx == 1)
                    os_ << '\'' << charset_ << '\'' << '\'';
                percent_encode(a, b);
                os_ << '\r' << '\n' << '\t';
                a = b;
            }
        }

        if (a != end) {
            os_ << key;
            if (idx)
                os_ << '*' << idx;
            os_ << '*' << '=';
            percent_encode(a, end);
        }
    }

    void percent_encode(string_iter beg, string_iter end)
    {
        std::string encoded = boost::locale::conv::from_utf(&*beg, &*end, charset_);
        for(auto ch : encoded) {
            if (std::isgraph(ch, loc)) {
                os_ << ch;
                ++cnt_;
            } else {
                char hex;
                os_ << '%';
                hex = (ch >> 4) & 0x0f;
                os_ << static_cast<char>((hex < 10) ? hex + '0' : hex + 'A' - 10);
                hex = ch & 0x0f;
                os_ << static_cast<char>((hex < 10) ? hex + '0' : hex + 'A' - 10);
                cnt_ += 3;
            }
        }
    }

    static std::string base64_encode(string_iter beg, string_iter end, std::string const & charset)
    {
        std::string str;
        do {
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(str));
            out << boost::locale::conv::from_utf(&*beg, &*end, charset);
        } while(false);
        return str;
    }

private:
    std::ostream & os_;
    std::string const & charset_;
    std::locale const & loc = std::locale::classic();
    std::ptrdiff_t cnt_ = 0;
    const std::ptrdiff_t max_ = 78;
};

} // detail


template <typename CharT>
inline void encode_mimeheader(std::ostream & os, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, std::string const & charset)
{
    detail::encode_mimeheader_impl<CharT> enc(os, charset);
    enc(key, val);
}


template <typename CharT, typename Params>
inline void encode_mimeheader(std::ostream & os, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, Params const & params, std::string const & charset)
{
    detail::encode_mimeheader_impl<CharT> enc(os, charset);
    enc(key, val)(params);
}

} }
