/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/email/decode_mimeheader.hpp>
#include <acqua/email/error.hpp>
#include <acqua/iostreams/ascii_filter.hpp>
#include <acqua/iostreams/qprint_filter.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/string_cast.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/asio/detail/throw_error.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/scope_exit.hpp>
#include <iostream>


namespace acqua { namespace email { namespace detail {

template <typename CharT>
inline std::basic_string<CharT> write_utf(std::string & str, std::string & charset, boost::system::error_code & ec)
{
    BOOST_SCOPE_EXIT_ALL(&str, &charset) { str.clear(); charset.clear(); };

    if (!charset.empty()) {
        try {
            return boost::locale::conv::to_utf<CharT>(str, charset);
        } catch(boost::locale::conv::conversion_error &) {
            ec = make_error_code(error::parse_aborted);
        } catch(boost::locale::conv::invalid_charset_error &) {
            ec = make_error_code(error::invalid_charset);
        }
    }
    return std::basic_string<CharT>();
}


template <typename It, typename CharT, typename Traits = std::char_traits<CharT>, typename String = std::basic_string<CharT, Traits>, typename Regex = boost::xpressive::basic_regex<String> >
inline void rfc2047_decode(It beg, It end, std::basic_ostream<CharT, Traits> & os, boost::system::error_code & ec, Regex const & regex)
{
    using xp_iter = boost::xpressive::regex_iterator<typename String::const_iterator>;

    std::locale const & loc = std::locale::classic();
    std::string encoded;
    std::string charset;

    auto a = beg;
    for(auto what = xp_iter(beg, end, regex); what != xp_iter(); ++what) {
        auto b = a;
        auto c = beg + what->position();
        while(b < c && std::isspace(*b, loc))
            ++b;
        if (b < c) {
            // 古いバッファをカキコ
            os << write_utf<CharT>(encoded, charset, ec);
            if (ec) return;
            std::copy(a, c, std::ostreambuf_iterator<CharT>(os));
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
        out << acqua::string_cast<std::string>(what->str(3));
        a = beg + what->position() + what->length();
    }
    // 古いバッファをカキコ
    os << write_utf<CharT>(encoded, charset, ec);
    if (ec) return;
    std::copy(a, end, std::ostreambuf_iterator<CharT>(os));
}


template <typename IIt, typename OIt>
inline void percent_decode(IIt it, IIt end, OIt out, boost::system::error_code & ec, std::locale const & loc)
{
    for(; it != end; ++it) {
        if (*it == '%') {
            char hex[] = {0,0,0};
            if (++it >= end && std::isxdigit(*it, loc)) {
                ec = make_error_code(error::parse_aborted);
                return;
            }
            hex[0] = *it;
            if (++it >= end && std::isxdigit(*it, loc)) {
                ec = make_error_code(error::parse_aborted);
                return;
            }
            hex[1] = *it;
            *out++ = static_cast<char>(std::strtol(hex, nullptr, 16));
        } else {
            *out++ = *it;
        }
    }
}


// val の文字コードを取り出して charset に格納し、val から文字コードを消す
template <typename String>
inline void rfc2231_parse_charset(String & val, std::string & charset)
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


template <typename CharT, typename It, typename Params, typename Regex>
inline void rfc2231_decode(It beg, It end, Params & params, boost::system::error_code & ec, Regex const & regex)
{
    std::locale const & loc = std::locale::classic();
    std::string oldkey;
    std::string encoded;
    std::string charset;

    while(beg != end) {
        if (std::isspace(*beg, loc) || *beg == ';') {
            ++beg;
            continue;
        }

        std::string newkey, val;
        boost::optional<boost::fusion::vector<boost::optional<int> > > idx;
        namespace qi = boost::spirit::qi;
        if (qi::parse(beg, end, +(qi::char_ - '*' - '=') >> -('*' >> -(qi::int_ >> '*')) >> '='
                      >> ('"' >> *(qi::char_ - '"') >> '"' | +(qi::char_ - qi::space)), newkey, idx, val)) {
            if (!idx) {
                if (!oldkey.empty()) {
                    // 古いバッファをカキコ
                    boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(params[oldkey]));
                    out << write_utf<CharT>(encoded, charset, ec);
                    if (ec) return;
                }

                // RFC2231 ではなく、プレーンテキストもしくはRFC2047エンコードされたテキスト
                boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(params[newkey]));
                rfc2047_decode(val.begin(), val.end(), out, ec, regex);
            } else {
                if (oldkey != newkey) {
                    if (!oldkey.empty()) {
                        // 古いバッファをカキコ
                        boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(params[oldkey]));
                        out << write_utf<CharT>(encoded, charset, ec);
                        if (ec) return;
                    }

                    oldkey = newkey;
                    if (charset.empty())
                        rfc2231_parse_charset(val, charset);
                }
                percent_decode(val.begin(), val.end(), std::back_inserter(encoded), ec, loc);
            }
        }
    }

    if (!oldkey.empty()) {
        // 古いバッファをカキコ
        boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(params[oldkey]));
        out << write_utf<CharT>(encoded, charset, ec);
    }
}


template <typename It>
inline void decode_mimeheader_impl(It beg, It end, std::string & str, boost::system::error_code & ec)
{

    namespace xp = boost::xpressive;
    xp::mark_tag s1(1), s2(2), s3(3);
    xp::sregex regex = xp::as_xpr('=') >> '?' >> (s1= +(xp::alnum|'_'|'='|'-') ) >> '?' >> (s2= (xp::as_xpr('B')|'b'|'Q'|'q')) >> '?' >> (s3= (*(xp::alnum|'+'|'_'|'/'|'-') >> -*xp::as_xpr('='))) >> '?' >> '=';
    boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(str));
    rfc2047_decode(beg, end, out, ec, regex);
}


template <typename It, typename Params>
inline void decode_mimeheader_impl(It beg, It end, std::string & str, Params & params, boost::system::error_code & ec)
{
    namespace xp = boost::xpressive;
    xp::mark_tag s1(1), s2(2), s3(3);
    xp::sregex regex = xp::as_xpr('=') >> '?' >> (s1= +(xp::alnum|'_'|'='|'-') ) >> '?' >> (s2= (xp::as_xpr('B')|'b'|'Q'|'q')) >> '?' >> (s3= (*(xp::alnum|'+'|'_'|'/'|'-') >> -*xp::as_xpr('='))) >> '?' >> '=';

    boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(str));
    auto it = std::find_if(beg, end, [](char v) { return v == ';' || v == ' ' || v == '\t'; });
    rfc2047_decode(beg, it, out, ec, regex);
    if (it != end)
        rfc2231_decode<char>(++it, end, params, ec, regex);
}


template <typename It>
inline void decode_mimeheader_impl(It beg, It end, std::wstring & str, boost::system::error_code & ec)
{
    namespace xp = boost::xpressive;
    xp::mark_tag s1(1), s2(2), s3(3);
    xp::wsregex regex = xp::as_xpr('=') >> '?' >> (s1= +(xp::alnum|'_'|'='|'-') ) >> '?' >> (s2= (xp::as_xpr('B')|'b'|'Q'|'q')) >> '?' >> (s3= (*(xp::alnum|'+'|'_'|'/'|'-') >> -*xp::as_xpr('='))) >> '?' >> '=';

    boost::iostreams::filtering_wostream out(boost::iostreams::back_inserter(str));
    rfc2047_decode(beg, end, out, ec, regex);
}


template <typename It, typename Params>
inline void decode_mimeheader_impl(It beg, It end, std::wstring & str, Params & params, boost::system::error_code & ec)
{
    namespace xp = boost::xpressive;
    xp::mark_tag s1(1), s2(2), s3(3);
    xp::wsregex regex = xp::as_xpr('=') >> '?' >> (s1= +(xp::alnum|'_'|'='|'-') ) >> '?' >> (s2= (xp::as_xpr('B')|'b'|'Q'|'q')) >> '?' >> (s3= (*(xp::alnum|'+'|'_'|'/'|'-') >> -*xp::as_xpr('='))) >> '?' >> '=';

    boost::iostreams::filtering_wostream out(boost::iostreams::back_inserter(str));
    auto it = std::find_if(beg, end, [](wchar_t v) { return v == ';' || v == ' ' || v == '\t'; });
    rfc2047_decode(beg, it, out, ec, regex);
    if (it != end)
        rfc2231_decode<wchar_t>(++it, end, params, ec, regex);
}

} // detail


template <typename It, typename CharT>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str)
{
    boost::system::error_code ec;
    detail::decode_mimeheader_impl(beg, end, str, ec);
    boost::asio::detail::throw_error(ec);
}


template <typename It, typename CharT>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str, boost::system::error_code & ec)
{
    detail::decode_mimeheader_impl(beg, end, str, ec);
}


template <typename It, typename CharT, typename Params>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str, Params & params)
{
    boost::system::error_code ec;
    detail::decode_mimeheader_impl(beg, end, str, params, ec);
    boost::asio::detail::throw_error(ec);
}


template <typename It, typename CharT, typename Params>
inline void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str, Params & params, boost::system::error_code & ec)
{
    detail::decode_mimeheader_impl(beg, end, str, params, ec);
}

} }
