#pragma once

#include <iterator>
#include <functional>
#include <boost/locale.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/text/base64.hpp>
#include <acqua/text/qprint.hpp>
#include <acqua/text/percent.hpp>

namespace acqua { namespace text {

class mime_header
{
public:
    template <typename It, typename CharT>
    static void decode(It beg, It end, std::basic_string<CharT> & out)
    {
        rfc2047_decode<CharT>(beg, end, std::back_inserter(out));
    }

    template <typename It, typename CharT, typename Map>
    static void decode(It beg, It end, std::basic_string<CharT> & out, Map & map)
    {
        auto it = std::find_if(beg, end, [](typename It::value_type const & v) { return v == ';' || v ==' ' || v == '\t'; });
        rfc2047_decode<CharT>(beg, it, std::back_inserter(out));
        if (it != end) rfc2231_decode<CharT>(++it, end, map);
    }

private:
    template <typename CharT, typename It, typename Out>
    static void rfc2047_decode(It beg, It end, Out out)
    {
        std::string encoded;
        std::string charset;

        namespace xp = boost::xpressive;
        xp::sregex regex = xp::sregex::compile("=\\?([[:alnum:]_=-]+)\\?([BbQq])\\?([[:alnum:]+_/=-]+)\\?=");

        auto a = beg;
        for(auto what = xp::sregex_iterator(beg, end, regex); what != xp::sregex_iterator(); ++what) {
            auto b = a;
            auto c = beg + what->position();
            while(b < c && std::isspace(*b, std::locale::classic())) ++b;
            if (b < c) {
                to_utf<CharT>(encoded, charset, out);
                std::copy(a, c, out);
                encoded.clear();
            }

            charset = what->str(1);
            switch(beg[what->position(2)]) {
                case 'B': case 'b':
                    base64::decode(what->str(3), encoded);
                    break;
                case 'Q': case 'q':
                    qprint::decode(what->str(3), encoded);
                    break;
            }
            a = beg + what->position() + what->length();
        }
        to_utf<CharT>(encoded, charset, out);
        std::copy(a, end, out);
    }

    template <typename CharT, typename It, typename Map>
    static void rfc2231_decode(It beg, It end, Map & map)
    {
        std::string charset;
        std::string key_;
        std::string encoded;

        namespace qi = boost::spirit::qi;
        while(beg != end) {
            while(std::isspace(*beg, std::locale::classic()))
                ++beg;

            std::string key;
            std::string val;
            boost::optional<boost::fusion::vector<boost::optional<int> > > idx;

            if (qi::parse(beg, end, +(qi::char_ - '*'-'=') >> -('*' >> -(qi::int_ >> '*')) >> '='
                          >> ('"' >> *(qi::char_ - '"') >> '"' | +(qi::char_ - qi::space)), key, idx, val)) {
                if (!idx) {
                    to_utf<CharT>(key_, encoded, charset, map);
                    rfc2047_decode<CharT>(val.begin(), val.end(), std::back_inserter(map[key]));
                } else {
                    if (key_ != key) {
                        to_utf<CharT>(key_, encoded, charset, map);
                        encoded.clear();

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
                    percent::decode(val, encoded);
                }
            }
        }

        to_utf<CharT>(key_, encoded, charset, map);
    }

    template <typename CharT, typename Out>
    static void to_utf(std::string const & in, std::string const & charset, Out & out)
    {
        if (charset.empty())
            boost::range::copy(in, out);
        else {
            try {
                boost::range::copy(boost::locale::conv::to_utf<CharT>(in, charset), out);
            } catch(...) {
                boost::range::copy(in, out);
            }
        }
    }

    template <typename CharT, typename Map>
    static void to_utf(std::string const & key, std::string const & val, std::string const & charset, Map & map)
    {
        if (!key.empty() && !val.empty()) {
            auto out = std::back_inserter(map[key]);
            to_utf<CharT>(val, charset, out);
        }
    }
};

} }
