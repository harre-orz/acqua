#pragma once

#include <locale>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <acqua/email/basic_address.hpp>

namespace acqua { namespace email { namespace utils {

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
namespace ascii = boost::spirit::ascii;

/*!
  アドレス型にをパースする.

  Skipper には ascii::space_type を指定することを推奨
 */
template <typename Iterator, typename Skipper>
class address_grammar
    : public qi::grammar<Iterator, acqua::email::basic_address<std::basic_string<typename Iterator::value_type> >(), Skipper>
{
    using input_type = std::vector<typename Iterator::value_type>;
    using output_type = acqua::email::basic_address<std::string>;

public:
    address_grammar()
        : address_grammar::base_type(rule_)
    {
        // このパーサは厳密なメールアドレスではなく、間違った形式でもパースできるようにしている
        rule_ =
            ( qi::no_skip[*(qi::char_ - '<')] >> '<' >> qi::no_skip[+(qi::char_ - '>')] >> '>')[phx::bind(&address_grammar::parse2, qi::_1, qi::_2, qi::_val)] |
            ( qi::no_skip[+(qi::char_ - ',')] )[phx::bind(&address_grammar::parse1, qi::_1, qi::_val)]
            ;
    }

    static void parse2(input_type const & name, input_type const & addr, output_type & result)
    {
        auto beg = name.begin();
        auto end = name.end();
        trim(beg, end, std::locale::classic());
        result.namespec.assign(beg, end);

        beg = addr.begin();
        end = addr.end();
        trim(beg, end, std::locale::classic());
        result.addrspec.assign(beg, end);
    }

    static void parse1(input_type const & addr, output_type & result)
    {
        auto beg = addr.begin();
        auto end = addr.end();
        trim(beg, end, std::locale::classic());
        result.addrspec.assign(beg, end);
    }

    template <typename It>
    static void trim(It & beg, It & end, std::locale const & locale)
    {
        while(beg != end && std::isspace(*beg, locale))
            ++beg;
        if (beg != end) {
            --end;
            while(beg != end && std::isspace(*end, locale))
                --end;
            ++end;
        }
    }

private:
    qi::rule<Iterator, output_type(), Skipper> rule_;
};


template <typename It, typename Addrs>
bool parse_addresses(It beg, It end, Addrs & addrs)
{
    address_grammar<It, ascii::space_type> g;
    return qi::phrase_parse(beg, end, g % ',', ascii::space, addrs);
}


} } }
