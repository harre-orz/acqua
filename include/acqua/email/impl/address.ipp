#pragma once

#include <type_traits>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/email/error.hpp>

namespace acqua { namespace email {

namespace detail {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename String, typename Skipper = boost::spirit::unused_type>
class address_grammar
    : public qi::grammar<Iterator, basic_address<String>(), Skipper>
{
private:
    struct trim
    {
        template <typename Context>
        void operator()(std::vector<char> const & vec, Context & ctx) const
        {
            auto beg = vec.begin();
            for(; beg != vec.end() && std::isspace(*beg); ++beg);
            auto end = vec.rbegin();
            for(; end != vec.rend() && std::isspace(*end); ++end);
            boost::fusion::at_c<0>(ctx.attributes).assign(beg, end.base());
        }
    };

public:
    address_grammar()
        : address_grammar::base_type(rule_)
    {
        space_ = qi::omit[*qi::space];
        name_ = ( *(qi::char_ - '<' - ',') )[trim()];
        addr_ = +(qi::char_ - '@') >> qi::char_('@') >> +(qi::char_("0-9a-zA-Z._-"));  // TODO: マルチバイト文字のドメイン名には対応していない
        rule_ = (
            (name_ >> '<' >> space_ >> addr_ >> space_ >> '>')
          | ( qi::attr(String()) >> space_ >> addr_)
        ) >> space_;
    }

private:
    qi::rule<Iterator, void(), Skipper> space_;
    qi::rule<Iterator, String(), Skipper> name_;
    qi::rule<Iterator, String(), Skipper> addr_;
    qi::rule<Iterator, basic_address<String>(), Skipper> rule_;
};

} // detail

template <typename String, typename It>
inline basic_address<String> make_address(It beg, It end, boost::system::error_code & ec)
{
    basic_address<String> addr;
    namespace qi = boost::spirit::qi;
    if (!qi::parse(beg, end, detail::address_grammar<It, String>(), addr) || beg != end) {
        ec = make_error_code(error::not_address);
        return basic_address<String>();
    }
    ec.clear();
    return addr;
}

template <typename It, typename Addresses>
inline std::size_t parse_to_addresses(It beg, It end, Addresses & addrs)
{
    using String = std::basic_string<typename std::iterator_traits<It>::value_type>;
    static_assert(std::is_same<typename Addresses::value_type, basic_address<String> >::value, "");

    // 最初の空白は削除
    while(beg != end && std::isspace(*beg))
        ++beg;
    // 最初のカンマは削除
    if (beg != end && *beg == ',')
        ++beg;
    auto size = addrs.size();
    namespace qi = boost::spirit::qi;
    qi::parse(beg, end, detail::address_grammar<It, String>() % ',', addrs);
    return (addrs.size() - size);
}

} }

BOOST_FUSION_ADAPT_TPL_STRUCT(
    (String),
    (acqua::email::basic_address)(String),
    (String, namespec)
    (String, addrspec)
)
