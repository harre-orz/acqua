#pragma once

#include <sstream>
#include <iterator>
#include <boost/locale.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/email/utils/qprint_decoder.hpp>
#include <acqua/email/utils/base64_decoder.hpp>

namespace acqua { namespace email { namespace utils {

class mime_header
{
public:
    template <typename String, typename CharT>
    static void encode(String & out, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val)
    {
        out += key;
        out += ": ";
        out += val;
    }

    template <typename String, typename CharT, typename Params>
    static void encode(String & out, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, Params const & params)
    {
        encode(out, key, val);
        for(auto const & kv : params) {
            out += ';';
            out += ' ';
            out += kv.first;
            out += '=';
            out += kv.second;
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
        auto it = std::find_if(beg, end, [](typename It::value_type const & v) { return v == ';' || v == ' ' || v == '\t'; });
        rfc2047_decode<CharT>(beg, it, std::back_inserter(out));
        if (it != end) rfc2231_decode<CharT>(it, end, params);
    }

private:
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
                                charset.assign(val.begin(), it1);
                                val.erase(val.begin(), ++it2);
                            }
                        }
                        key_ = key;
                    }
                    percent_decode(val, oss);
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

    static void percent_decode(std::string const & in, std::ostream & os)
    {
        auto end = in.end();
        for(auto it = in.begin(); it != end; ++it) {
            if (*it == '%') {
                char hex[] = {0,0,0};
                if (++it >= end && std::isxdigit(*it, std::locale::classic()))
                    return;
                hex[0] = *it;
                if (++it >= end && std::isxdigit(*it, std::locale::classic()))
                    return;
                hex[1] = *it;
                os << (char)std::strtol(hex, nullptr, 16);
            } else {
                os << *it;
            }
        }
    }
};

} } }
