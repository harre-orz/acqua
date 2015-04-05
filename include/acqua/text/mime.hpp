#pragma once

#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include <list>

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/locale.hpp>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>

#include <acqua/string_cast.hpp>
#include <acqua/text/base64.hpp>
#include <acqua/text/quoted_printable.hpp>
#include <acqua/text/percent_encoding.hpp>

namespace acqua { namespace text {

//! MIME変換クラス
class mime
{
public:
    //! MIMEヘッダー形式で符号化されているデータを通常の文字列に変換する
    template <typename String>
    static void decode_mimeheader(String & text)
    {
        namespace xp = boost::xpressive;
        typedef typename String::const_iterator BidiIter;
        typedef xp::basic_regex<BidiIter> regex_type;
        typedef xp::match_results<BidiIter> match_type;

        regex_type regex = regex_type::compile(acqua::string_cast<String>("=\\?([[:alnum:]_=-]+)\\?([[:alnum:]]+)\\?([[:alnum:]+_/=-]+)\\?="));
        match_type what;
        std::size_t end_pos;
        std::string charset, temp;

        while(xp::regex_search(text, what, regex)) {
            charset = acqua::string_cast<decltype(charset)>(what.str(1));
            auto const & match = what[3];

            switch(text[ what.position(2) ]) {
                case 'B': case 'b':
                    base64::decode(match.first, match.second, std::back_inserter(temp));
                    break;
                case 'Q': case 'q':
                    quoted_printable::decode(match.first, match.second, std::back_inserter(temp));
                    break;
                default:
                    std::copy(match.first, match.second, std::back_inserter(temp));
                    break;
            }

            auto const & matches = what[0];
            if ((end_pos = text.find_first_not_of(acqua::string_cast<String>("; \t\r\n"), matches.second - matches.first)) == text.npos)
                end_pos = what[0].second - what[0].first;

            try {
                text.replace(
                    text.begin() + what.position(0),
                    text.begin() + end_pos,
                    boost::locale::conv::to_utf<typename String::value_type>(temp, charset)
                );
            } catch(boost::locale::conv::invalid_charset_error & ex) {
                text.replace(
                    text.begin() + what.position(0),
                    text.begin() + end_pos,
                    boost::locale::conv::utf_to_utf<typename String::value_type>(temp)
                );
            }

            temp.clear();
        }
    }

    template <typename String>
    static String decode_mimeheader_copy(String const & text)
    {
        String ret = text;
        decode_mimeheader(ret);
        return ret;
    }

    //! 通常の文字列をMIMEヘッダー形式の文字列に変換する
    template <typename TransferEncodingCategory = base64, typename LineWrapCategory = no_line_wrap, typename String>
    static void encode_mimeheader(String & text, std::string const & charset)
    {
        // ASCIIコードのみのときは、変換不要
        if (text.size() < LineWrapCategory::count && is_ascii_string(text)) {
            if (text.size() > 0 && (std::isspace(text[0], std::locale("C")) || std::isspace(text[text.size()-1], std::locale("C"))))
                ;  // 文字の端が空白文字のときは、変換する
            else
                return;
        }

        // まず、文字コードを変換しつつ、LineWrapCategory により複数行に分ける
        String temp = TransferEncodingCategory::template encode<String, LineWrapCategory>(
            boost::locale::conv::from_utf<typename String::value_type>(text, charset)
        );

        text.clear();
        int break_point = 2;  // 最初は '\t' を避ける
        for(auto it = temp.begin(); it != temp.end(); text += *it++) {
            switch(*it) {
                case '\r': case '\n':
                    if (break_point == 0) {
                        text += '?';
                        text += '=';
                        text += ';';
                    }

                    break_point = 1;
                    break;
                default:
                    if (break_point != 0) {
                        if (break_point == 1) {
                            text += '\t';  // ';' の直後に追加すると改行コードの前に '\t' が挿入されてしまうので、ここに書く必要がある
                        }
                        break_point = 0;
                        text += '=';
                        text += '?';
                        text.append(charset.begin(), charset.end());
                        text += '?';
                        text += transfer_symbol(TransferEncodingCategory());
                        text += '?';
                    }
                    break;
            }
        }

        if (break_point == 0) {
            text += '?';
            text += '=';
        }
    }

    template <typename TransferEncodingCategory = base64, typename LineWrapCategory = no_line_wrap, typename String>
    static String encode_mimeheader_copy(String const & text, std::string const & charset)
    {
        String ret = text;
        encode_mimeheader<TransferEncodingCategory, LineWrapCategory>(ret, charset);
        return ret;
    }

private:
    static char transfer_symbol(base64 const &) { return 'B'; }
    static char transfer_symbol(quoted_printable const &) { return 'Q'; }

public:

    //! MIMEのdisp_param である text をパースして value と params に格納する
    template <typename String, typename Value, typename Map>
    static bool parse_param(String const & text, Value & value, Map & params)
    {
        using acqua::string_cast;
        namespace qi = boost::spirit::qi;
        namespace xp = boost::xpressive;

        value.clear();
        params.clear();

        auto a = text.begin(), end = text.end();
        if (!qi::parse(a, end, qi::omit[*qi::space] >> *(qi::char_ - ';'-'\r'-'\n'), value))
            return false;

        String param;
        boost::container::flat_map<int, std::string> temp;
        boost::optional<int> idx;
        typename Map::key_type key, param_key;
        std::string charset;
        char quote;

        /*
         * key はqi::parse で得られたキー名で param_key はparams に格納するときのキー名。
         * この２つの変数が一致していると同じキー名が継続する可能性があるため temp に蓄え、
         * 不一致になったときに temp を params[キー名] に書き込む
         */
        while (qi::parse(a, end, qi::omit[*qi::char_("; \t\r\n")] >> +(qi::char_ - '*'-'=') >> -('*' >> -(qi::int_ >> '*')) >> '=' >> qi::char_, key, idx, quote)) {

            if (!param_key.empty() && param_key != key) {
                params[param_key] = concat_decoded_param<Value>(temp, charset);
                temp.clear();
                charset.clear();
            }

            auto & param = temp[idx ? *idx : 0];
            decltype(a) b;

            if (quote == '"' && (b = std::find(a, end, quote)) != end) {
                param.append(a, b++);
            } else {
                --a;
                const char sep[] = "; \t\r\n";
                b = std::find_first_of(a, end, sep, sep + sizeof(sep)/sizeof(*sep) - 1);
                param.append(a, b);
            }

            decode_mimeheader(param);

            xp::mark_tag charset_(1);
            xp::sregex regex = xp::bos >> (charset_ = *xp::set[xp::graph] >> xp::before('\'')) >> '\'' >> *xp::set[xp::graph] >> xp::before('\'') >> '\'';
            //xp::sregex regex = xp::sregex::compile("^([[:alnum:]_=-]*)'[[:alnum:]_=-]*'");
            xp::smatch what;

            if (xp::regex_search(param, what, regex)) {
                charset = what[charset_].str();
                param.erase(0, what.length(0));
            }
            acqua::text::percent_encoding::decode(param);
            param_key.swap(key);
            key.clear();
            idx = boost::none;
            a = b;
        }

        if (!param_key.empty()) {
            params[param_key] = concat_decoded_param<Value>(temp, charset);
        }

        return true;
    }

    /*!
     * key, value, map を MIMEのdisp_param である文字列に結合する
     */
    template <
        typename LineWrapCategory = line_wrap<37, boost::mpl::list_c<char, ';', '\r', '\n'> >,
        typename OS,
        typename Key,
        typename Value,
        typename Map
        >
    static void join_param(OS & os, Key const & key, Value const & value, Map const & map, std::string const & charset, LineWrapCategory lineNo = LineWrapCategory())
    {
        typedef std::basic_string<typename OS::char_type> string_type;

        os << acqua::string_cast(key)
           << ':' << ' ';
        lineNo += key.size() + 2;

        if (is_ascii_string(value)) {
            os << acqua::string_cast(value);
            lineNo += value.size();
        } else {
            string_type temp = acqua::string_cast<string_type>(value);
            encode_mimeheader<base64, LineWrapCategory>(temp, charset);
            os << temp;
            lineNo += temp.size();
        }

        if (!map.empty()) {
            auto name_it = map.find(acqua::string_cast<typename Map::key_type>("name"));

            for(auto it = map.begin(); it != map.end(); ++it) {
                if (it->second.empty())
                    continue;

                bool is_msoe_flag = (boost::iequals(key, "Content-Type") && it == name_it);

                if (is_ascii_string(it->second)) {
                    lineNo += it->first.size() + it->second.size() + 1;
                    os << (lineNo(os) ? '\t' : ' ');
                    os << acqua::string_cast(it->first) << '=';
                    if (is_msoe_flag) os << '\"';
                    os << acqua::string_cast(it->second);
                    if (is_msoe_flag) os << '\"';

                } else {
                    (lineNo += lineNo.count)(os);
                    os << '\t';
                    if (is_msoe_flag) {
                        string_type temp = acqua::string_cast<string_type>(it->second);
                        encode_mimeheader<base64, LineWrapCategory>(temp, charset);
                        os <<acqua::string_cast(it->first) << '=' << '"' << temp << '"';
                        lineNo += it->first.size() + temp.size() + 3;
                    } else {
                        string_type temp = acqua::text::percent_encoding::encode<string_type>(boost::locale::conv::from_utf(it->second, charset));
                        auto beg = temp.begin(), end = beg + std::min(lineNo.count - (it->first.size() + charset.size() + 6), temp.size());
                        if (end == temp.end()) {
                            os << acqua::string_cast(it->first) << '*' << '='
                               << acqua::string_cast(charset) << '\'' << '\''
                               << acqua::string_cast(temp);
                        } else {
                            int i = 0;
                            while(beg != temp.end()) {
                                os << acqua::string_cast(it->first) << '*' << i++ << '=';
                                if (beg == temp.begin())
                                    os << acqua::string_cast(charset) << '\'' << '\'';
                                os << acqua::string_cast(beg, end);

                                (lineNo += lineNo.count)(os);
                                os << '\t';
                                beg = end;
                                if ((end += LineWrapCategory::count) > temp.end())
                                    end = temp.end();
                            }
                        }
                    }
                }
            }
        }
    }

private:
    /*!
     * デコード済みパラメータを連結してから、文字コード変換を行う
     * 先に文字コードを変換してしまうと、文字データ内の中途半端な位置で区切られた時に、デコードできないため
     */
    template <typename String, typename Map>
    static String concat_decoded_param(Map & map, std::string const & charset)
    {
        auto & param = map[0];
        for(auto it = std::next(map.begin()); it != map.end(); ++it)
            param += it->second;

        try {
            return boost::locale::conv::to_utf<typename String::value_type>(param, charset);
        } catch(boost::locale::conv::invalid_charset_error & ex) {
        }

        return boost::locale::conv::utf_to_utf<typename String::value_type>(param);
    }

    template <typename String>
    static bool is_ascii_string(String const & str)
    {
        for(auto const & _ : str)
            if (!std::isprint(_, std::locale("C")))
                return false;
        return true;
    }
};

} }
