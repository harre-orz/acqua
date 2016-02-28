#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/email/address.hpp>
#include <acqua/email/error.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/system/system_error.hpp>
#include <type_traits>

namespace acqua { namespace email {

namespace detail {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename String, typename Skipper = boost::spirit::unused_type>
class addrspec_grammar
    : public qi::grammar<Iterator, String(), Skipper>
{
public:
    addrspec_grammar()
        : addrspec_grammar::base_type(rule_)
    {
        // TODO: マルチバイト文字のドメイン名には対応していない
        rule_ = +(qi::char_ - '@') >> qi::char_('@') >> +(qi::char_("0-9a-zA-Z._-"));
    }

private:
    qi::rule<Iterator, String(), Skipper> rule_;
};


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
        rule_ = (
            (name_ >> '<' >> space_ >> addr_ >> space_ >> '>')
          | ( qi::attr(String()) >> space_ >> addr_)
        ) >> space_;
    }

private:
    qi::rule<Iterator, void(), Skipper> space_;
    qi::rule<Iterator, String(), Skipper> name_;
    addrspec_grammar<Iterator, String, Skipper> addr_;
    qi::rule<Iterator, basic_address<String>(), Skipper> rule_;
};

} // detail


template <typename String>
inline bool basic_address<String>::is_valid() const
{
    auto beg = addrspec.begin(), end = addrspec.end();
    return (boost::spirit::qi::parse(beg, end, detail::addrspec_grammar<decltype(beg), String>()) && beg == end);
}


template <typename String>
inline basic_address<String> basic_address<String>::from_string(String const & str)
{
    boost::system::error_code ec;
    auto addr = basic_address<String>::from_string(str, ec);
    if (ec) throw boost::system::system_error(ec);
    return addr;
}


template <typename String>
inline basic_address<String> basic_address<String>::from_string(String const & str, boost::system::error_code & ec)
{
    basic_address<String> addr;
    auto beg = str.begin(), end = str.end();
    if (!boost::spirit::qi::parse(beg, end, detail::address_grammar<decltype(beg), String>(), addr) || beg != end) {
        ec = make_error_code(error::not_address);
        return basic_address<String>();
    }
    return addr;
}


template <typename It, typename Addresses>
inline std::size_t parse_to_addresses(It beg, It end, Addresses & addrs)
{
    using String = std::basic_string<typename std::iterator_traits<It>::value_type>;
    static_assert(std::is_same<typename Addresses::value_type, basic_address<String> >::value, "");

    // 最初の空白は飛ばす
    while(beg != end && std::isspace(*beg))
        ++beg;
    // 最初のカンマは飛ばす
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
