#pragma once

#include <sstream>
#include <iterator>
#include <boost/locale.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/email/utils/qprint_decoder.hpp>
#include <acqua/email/utils/base64_decoder.hpp>
#include <acqua/email/utils/qprint_encoder.hpp>
#include <acqua/email/utils/base64_encoder.hpp>


namespace acqua { namespace email { namespace utils {

class mime_header
{
public:
    template <typename String, typename CharT>
    static std::size_t encode(String & out, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val)
    {
        std::locale locale = std::locale::classic();
        if (!append_key(key, out))
            return 0;
        out += ": ";
        return is_ascii_string(val, locale)
            ? append_ascii_value(val, out, locale)
            : append_rfc2047_value(val, out, locale);
    }

    template <typename String, typename CharT, typename Params>
    static void encode(String & out, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, Params const & params)
    {
        std::locale locale = std::locale::classic();
        std::size_t length = encode(out, key, val);
        if (length) {
            for(auto const & kv : params) {
                if (is_ascii_string(kv.second, locale)) {
                    if (length > 78) {
                        length = 1;
                        out += ";\r\n ";
                    } else {
                        out += "; ";
                        length += 2;
                    }
                    out += kv.first;
                    out += '=';
                    out += kv.second;
                    length += kv.first.size() + kv.second.size() + 1;
                } else {
                    out += ";\r\n ";
                    length += append_rfc2231_value(kv.first, kv.second, out, locale, 78) + kv.first.size() + 2;
                }
            }
        }
    }

    template <typename It, typename CharT>
    static void decode(It beg, It end, std::basic_string<CharT> & out)
    {
        rfc2047_decode<CharT>(beg, end, std::back_inserter(out));
    }

    template <typename It, typename CharT, typename Params>
    static void decode(It beg, It end, std::basic_string<CharT> & out, Params & params)
    {
        auto it = std::find_if(beg, end, [](auto const & v) { return v == ';' || v == ' ' || v == '\t'; });
        rfc2047_decode<CharT>(beg, it, std::back_inserter(out));
        if (it != end) rfc2231_decode<CharT>(it, end, params);
    }

private:
    template <typename String>
    static bool is_ascii_string(String const & str, std::locale const & locale)
    {
        return std::find_if(str.begin(), str.end(), [&locale](auto const & ch) { return !std::isprint(ch, locale); }) == str.end();
    }

    template <typename In, typename Out>
    static std::size_t append_key(In const & key, Out & out)
    {
        for(auto const & ch : key)
            if (std::isalnum(ch, std::locale::classic()) || ch == '-')
                out += ch;
        return out.size();
    }

    template <typename In, typename Out>
    static std::size_t append_ascii_value(In const & val, Out & out, std::locale const & locale, std::size_t line_break = 78)
    {
        // line_break を超えないようにしつつ val の空白文字で区切る
        std::size_t length = out.size();
        auto a = val.begin();
        // 先頭に空白文字がある場合は飛ばす
        auto b = std::find_if(a, val.end(), [&locale](auto const & ch) { return !std::isspace(ch, locale); });
        b = std::find_if(b, val.end(), [&locale](auto const & ch) { return std::isspace(ch, locale); });
        while(b != val.end()) {
            // 次の空白でない文字を探す
            auto c = std::find_if(b, val.end(), [&locale](auto const & ch) { return !std::isspace(ch, locale); });
            if (c == val.end()) {
                // 文字がないときは、空白文字も書き出して終わり
                b = c;
                break;
            }

            // さらに次の空白もしくは文字の終端を探す
            auto d = std::find_if(c, val.end(), [&locale](auto const & ch) { return std::isspace(ch, locale); });
            if (length + (d - a) < line_break) {
                out.append(a, c);
                length += (c - a);
            } else {
                out.append(a, b);
                out += "\r\n";
                out.append(b, c);
                length = (c - b);
            }
            a = c;
            b = d;
        }

        if ((a == val.begin()) && (length + val.size()) > line_break) {
            // 無変換で folding できないときは rfc2047変換へ
            return append_rfc2047_value(val, out, locale, line_break);
        }

        out.append(a, b);
        length += (b - a);
        return length;
    }

    template <typename In, typename Out>
    static std::size_t append_rfc2047_value(In const & val, Out & out, std::locale const & locale, std::size_t line_break = 78)
    {
        using utf = boost::locale::utf::utf_traits<typename In::value_type>;
        auto a = val.begin();
        for(auto b = val.begin(); b != val.end();) {
            switch(utf::width(*b)) {
                case 4:
                    if (++b == val.end())
                        break;
                case 3:
                    if (++b == val.end())
                        break;
                case 2:
                    if (++b == val.end())
                        break;
                case 1:
                    if (++b == val.end())
                        break;
            }
            if ((b - a) > (std::ptrdiff_t)line_break / 2) {
                out += "=?ISO-2022-JP?B?";
                acqua::email::utils::base64_encoder enc("ISO-2022-JP");
                enc.read_one(std::string(a, b), out);
                a = b;
                out += "?=\r\n ";
            }
        }

        if (a != val.end()) {
            out += "=?ISO-2022-JP?B?";
            acqua::email::utils::base64_encoder enc("ISO-2022-JP");
            enc.read_one(std::string(a, val.end()), out);
            out += "?=";
        }

        return 0;
    }

    template <typename K, typename V, typename Out>
    static std::size_t append_rfc2231_value(K const & key, V const & val, Out & out, std::locale const & locale, std::size_t line_break)
    {
        using utf = boost::locale::utf::utf_traits<typename V::value_type>;
        std::size_t count = 0;
        auto a = val.begin();
        for(auto b = val.begin(); b != val.end();) {
            switch(utf::width(*b)) {
                case 4:
                    if (++b == val.end())
                        break;
                case 3:
                    if (++b == val.end())
                        break;
                case 2:
                    if (++b == val.end())
                        break;
                case 1:
                    if (++b == val.end())
                        break;
            }
            if ((b - a) > (std::ptrdiff_t)line_break / 2) {
                out += key;
                out += "*";
                out += boost::lexical_cast<std::string>(++count);
                out += "*=";
                if (count == 1) {
                    out += "'ISO-2022-JP''";
                }
                percent_encode(a, b, out, locale);
                a = b;
                out += "\r\n ";
            }
        }

        if (a != val.end()) {
            out += key;
            if (count != 0) {
                out += "*";
                out += boost::lexical_cast<std::string>(count);
            }
            out += "*=";
            percent_encode(a, val.end(), out, locale);
        }

        return 0;
    }

    template <typename CharT, typename It, typename Out>
    static void rfc2047_decode(It beg, It end, Out out)
    {
        std::ostringstream oss;
        std::string charset;

        namespace xp = boost::xpressive;
        xp::sregex regex = xp::sregex::compile("=\\?([[:alnum:]_=-]+)\\?([BbQq])\\?([[:alnum:]+_/=-]+)\\?=");

        auto a = beg;
        for(auto what = xp::sregex_iterator(beg, end, regex); what != xp::sregex_iterator(); ++what) {
            auto b = a;
            auto c = beg + what->position();
            while(b < c && std::isspace(*b, std::locale::classic())) ++b;
            if (b < c) {
                to_utf<CharT>(oss, charset, out);
                std::copy(a, c, out);
            }

            charset = what->str(1);
            switch(beg[what->position(2)]) {
                case 'B': case 'b':  {
                    base64_decoder dec;
                    dec.write(oss, what->str(3));
                    dec.flush(oss);
                    break;
                }
                case 'Q': case 'q': {
                    qprint_decoder dec;
                    dec.write(oss, what->str(3));
                    dec.flush(oss);
                    break;
                }
            }
            a = beg + what->position() + what->length();
        }
        to_utf<CharT>(oss, charset, out);
        std::copy(a, end, out);
    }

    template <typename CharT, typename It, typename Params>
    static void rfc2231_decode(It beg, It end, Params & params)
    {
        std::string charset;
        std::string key_;
        std::ostringstream oss;

        while(beg != end) {
            if (std::isspace(*beg, std::locale::classic()) || *beg == ';') {
                ++beg;
                continue;
            }

            std::string key;
            std::string val;
            boost::optional<boost::fusion::vector<boost::optional<int> > > idx;

            namespace qi = boost::spirit::qi;
            if (qi::parse(beg, end, +(qi::char_ - '*' - '=') >> -('*' >> -(qi::int_ >> '*')) >> '='
                          >> ('"' >> *(qi::char_ - '"') >> '"' | +(qi::char_ - qi::space)), key, idx, val)) {
                if (!idx) {
                    to_utf<CharT>(key_, oss, charset, params);
                    rfc2047_decode<CharT>(val.begin(), val.end(), std::back_inserter(params[key]));
                } else {
                    if (key_ != key) {
                        to_utf<CharT>(key_, oss, charset, params);
                        auto it1 = std::find(val.begin(), val.end(), '\'');
                        if (it1 != val.end()) {
                            auto it2 = std::find(++it1, val.end(), '\'');
                            if (it2 != val.end()) {
                                it2 = std::find(++it2, val.end(), '\'');
                                if (it2 != val.end()) {
                                    charset.assign(val.begin(), it1);
                                    val.erase(val.begin(), ++it2);
                                }
                            }
                        }
                        key_ = key;
                    }
                    percent_decode(val.begin(), val.end(), oss, std::locale::classic());
                }
            }
        }
        to_utf<CharT>(key_, oss, charset, params);
    }

    template <typename CharT, typename Out>
    static void to_utf(std::ostringstream & in, std::string const & charset, Out & out)
    {
        std::string str = in.str();
        in.str("");

        if (charset.empty())
            boost::range::copy(str, out);
        else {
            try {
                boost::range::copy(boost::locale::conv::to_utf<CharT>(str, charset), out);
            } catch(...) {
                boost::range::copy(str, out);
            }
        }
    }

    template <typename CharT, typename Params>
    static void to_utf(std::string const & key, std::ostringstream & val, std::string const & charset, Params & params)
    {
        if (!key.empty()) {
            auto out = std::back_inserter(params[key]);
            to_utf<CharT>(val, charset, out);
        }
    }

    template <typename It>
    static void percent_decode(It it, It end, std::ostream & os, std::locale const & locale)
    {
        while(it != end) {
            if (*it == '%') {
                char hex[] = {0,0,0};
                if (++it >= end && std::isxdigit(*it, locale))
                    return;
                hex[0] = *it;
                if (++it >= end && std::isxdigit(*it, locale))
                    return;
                hex[1] = *it;
                os << (char)std::strtol(hex, nullptr, 16);
            } else {
                os << *it;
            }
            ++it;
        }
    }

    template <typename It>
    static std::size_t percent_encode(It it, It end, std::string & out, std::locale const & locale)
    {
        std::size_t size = out.size();
        while(it != end) {
            if (std::isgraph(*it, locale)) {
                out += *it;
            } else {
                char hex;
                out += '%';
                hex = (*it >> 4) & 0x0f;
                out += (hex < 10) ? hex + '0': hex + 'A' - 10;
                hex = (*it) & 0x0f;
                out += (hex < 10) ? hex + '0': hex + 'A' - 10;
            }
            ++it;
        }
        return out.size() - size;
    }
};

} } }
