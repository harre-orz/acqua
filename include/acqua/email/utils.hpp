#pragma once

#include <boost/spirit/include/qi.hpp>
#include <acqua/email/basic_address.hpp>

namespace acqua { namespace email { namespace utils {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename Skipper>
class address_grammar
    : public qi::grammar<Iterator, acqua::email::basic_address<std::string>(), Skipper>
{
public:
    address_grammar()
        : address_grammar::base_type(rule_)
    {
    }

private:
    qi::rule<Iterator, acqua::email::basic_address<String>(), Skipper> rule_;
};


/*!
  From, To, Cc などのヘッダー文字列パースして、アドレスリストに格納する.
 */
template <typename It, typename Addresses>
inline std::size_t parse_address(It beg, It end, Addresses & addrs)
{
    static_assert(std::is_same<typename std::iterator_traits<It>::value_type, char>::value, "");

    auto it = beg;
    std::string addr, name;

    namespace qi = boost::spirit::qi;
    while (it != end && qi::parse(it, end, (
            ( +(qi::char_ - '<') >> '<' >> qi::omit[*qi::space] >> +(qi::char_ - '>') >> '>' >> qi::omit[*qi::space] ) |
            ( qi::attr(std::string()) >> qi::omit[*qi::space] >> +(qi::char_ - ',') ) ) >> -qi::lit(','), name, addr)) {
        boost::trim_right(addr);
        boost::trim_right(name);
        addrs.emplace_back(addr, name);
        addr.clear();
        name.clear();
    }
    return it - beg;
}

template <typename Addresses>
inline std::size_t parse_address(std::string const & text, Addresses & addrs)
{
    return parse_address(text.begin(), text.end(), addrs);
}


} } }
