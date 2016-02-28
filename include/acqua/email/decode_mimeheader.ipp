#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/email/decode_mimeheader.hpp>
#include <acqua/email/error.hpp>
#include <acqua/iostreams/ascii_filter.hpp>
#include <acqua/iostreams/qprint_filter.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/utility/string_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/optional.hpp>
#include <iostream>

namespace acqua { namespace email { namespace detail {

template <typename It, typename CharT>
class decode_mimeheader_impl
{
public:
    using string_type = std::basic_string<CharT>;

private:
    using string_iter = typename string_type::const_iterator;
    using regex_type = boost::xpressive::basic_regex<string_iter>;
    using regex_iter = boost::xpressive::regex_iterator<string_iter>;
    using ostream_type = std::basic_ostream<CharT>;
    using filtering_ostream_type = typename std::conditional<
        std::is_same<CharT, char>::value,
        boost::iostreams::filtering_ostream,
        boost::iostreams::filtering_wostream
        >::type;

public:
    explicit decode_mimeheader_impl(string_type & str)
        : out_(boost::iostreams::back_inserter(str))
        , s1(1), s2(2), s3(3)
        , regex(boost::xpressive::as_xpr('=') >> '?' >>
                (s1= +(boost::xpressive::alnum|'_'|'='|'-') ) >> '?' >>
                (s2= (boost::xpressive::as_xpr('B')|'b'|'Q'|'q')) >> '?' >>
                (s3= (*(boost::xpressive::alnum|'+'|'_'|'/'|'-') >> -*boost::xpressive::as_xpr('=')))
                >> '?' >> '=') {}

    decode_mimeheader_impl & operator()(It beg, It end)
    {
        rfc2047_decode(out_, beg, end);
        return *this;
    }

    template <typename Params>
    decode_mimeheader_impl & operator()(It beg, It end, Params & params)
    {
        It it = std::find_if(beg, end, [](CharT ch) { return ch == ';' || ch == ' ' || ch == '\t'; });
        rfc2047_decode(out_, beg, it);
        if (it != end)
            rfc2231_decode(++it, end, params);
        return *this;
    }

private:
    static void write_utf(string_type & str, std::string & encoded, std::string & charset)
    {
        str = boost::locale::conv::to_utf<CharT>(encoded, charset);
        encoded.clear();
        charset.clear();
    }

    static void write_utf(ostream_type & os, std::string & encoded, std::string & charset)
    {
        os << boost::locale::conv::to_utf<CharT>(encoded, charset);
        encoded.clear();
        charset.clear();
    }

    static void write_raw(ostream_type & os, string_iter beg, string_iter end)
    {
        std::copy(beg, end, std::ostreambuf_iterator<CharT>(os));
    }

    void rfc2047_decode(ostream_type & os, It beg, It end)
    {
        std::string encoded;
        std::string charset;

        auto a = beg;
        for(auto what = regex_iter(beg, end, regex); what != regex_iter(); ++what) {
            auto b = a;
            auto c = beg + what->position();
            while(b < c && std::isspace(*b, loc))
                ++b;
            if (b < c) {
                // 古いバッファをカキコ
                write_utf(os, encoded, charset);
                write_raw(os, a, c);
            }

            charset = acqua::string_cast<std::string>(what->str(1));
            boost::iostreams::filtering_ostream out;
            switch(beg[what->position(2)]) {
                case 'B': case 'b':
                    out.push(acqua::iostreams::base64_decoder());
                    break;
                case 'Q': case 'q':
                    out.push(acqua::iostreams::qprint_decoder());
                    break;
            }
            out.push(boost::iostreams::back_inserter(encoded));
            out << acqua::string_cast(what->str(3));
            a = beg + what->position() + what->length();
        }
        // 古いバッファをカキコ
        write_utf(os, encoded, charset);
        write_raw(os, a, end);
    }

    void percent_decode(string_iter it, string_iter end, std::string & out) const
    {
        for(; it != end; ++it) {
            if (*it == '%') {
                char hex[] = {0,0,0};
                if (++it >= end && std::isxdigit(*it, loc))
                    BOOST_THROW_EXCEPTION( syntax_error() );
                hex[0] = static_cast<char>(*it);
                if (++it >= end && std::isxdigit(*it, loc))
                    BOOST_THROW_EXCEPTION( syntax_error() );
                hex[1] = static_cast<char>(*it);
                out.push_back( static_cast<char>(std::strtol(hex, nullptr, 16)) );
            } else {
                out.push_back( static_cast<char>(*it) );
            }
        }
    }

    template <typename Params>
    void rfc2231_decode(It beg, It end, Params & params)
    {
        string_type oldkey;
        std::string encoded, charset;

        while(beg != end) {
            if (std::isspace(*beg, loc) || *beg == ';') {
                ++beg;
                continue;
            }

            string_type newkey, val;
            boost::optional<boost::fusion::vector<boost::optional<int> > > idx;
            namespace qi = boost::spirit::qi;
            if (qi::parse(beg, end, +(qi::char_ - '*' - '=') >> -('*' >> -(qi::int_ >> '*')) >> '='
                          >> ('"' >> *(qi::char_ - '"') >> '"' | +(qi::char_ - qi::space)), newkey, idx, val)) {
                if (!idx) {
                    // 古いバッファをカキコ
                    if (!oldkey.empty())
                        write_utf(params[oldkey], encoded, charset);

                    // RFC2231 ではなく、プレーンテキストもしくはRFC2047エンコードされたテキスト
                    filtering_ostream_type out(boost::iostreams::back_inserter(params[newkey]));
                    rfc2047_decode(out, val.begin(), val.end());
                } else {
                    if (oldkey != newkey) {
                        // 古いバッファをカキコ
                        if (!oldkey.empty())
                            write_utf(params[oldkey], encoded, charset);
                        oldkey = newkey;
                        if (charset.empty())
                            rfc2231_parse_charset(val, charset);
                    }
                    percent_decode(val.begin(), val.end(), encoded);
                }
            }
        }

        // 古いバッファをカキコ
        if (!oldkey.empty())
            write_utf(params[oldkey], encoded, charset);
    }

    // val の文字コードを取り出して charset に格納し、val から文字コードを消す
    static void rfc2231_parse_charset(string_type & val, std::string & charset)
    {
        auto end = val.end();
        auto a = std::find(val.begin(), end, '\'');
        if (a != end) {
            auto b = std::find(++a, end, '\'');
            if (b != end) {
                auto c = std::find(b+1, end, '\'');
                if (c != end) {
                    charset.assign(a, b);
                    val.erase(val.begin(), ++c);
                    return;
                }
            }
        }
        charset = "US-ASCII";
    }

private:
    filtering_ostream_type out_;
    boost::xpressive::mark_tag s1, s2, s3;
    const regex_type regex;
    std::locale const & loc = std::locale::classic();
};

} // detail

template <typename It, typename CharT>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str)
{
    detail::decode_mimeheader_impl<It, CharT> dec(str);
    dec(beg, end);
}

template <typename It, typename CharT, typename Params>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str, Params & params)
{
    detail::decode_mimeheader_impl<It, CharT> dec(str);
    dec(beg, end, params);
}

} }
