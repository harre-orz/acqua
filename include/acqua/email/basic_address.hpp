#pragma once

#include <iterator>
#include <boost/algorithm/string/trim.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/assign.hpp>
#include <boost/assign/list_inserter.hpp>

namespace acqua { namespace email {

/*!
  メールアドレスクラス.
 */
template <typename String>
class basic_address
{
public:
    using value_type = String;

    basic_address() = default;

    basic_address(value_type const & addrspec)
        : namespec(), addrspec(addrspec) {}

    basic_address(value_type const & addrspec, value_type const & namespec)
        : namespec(namespec), addrspec(addrspec) {}

public:
    value_type namespec;
    value_type addrspec;
};

// /*!
//   From, To, Cc などのヘッダー文字列パースして、アドレスリストに格納する.
//  */
// template <typename It, typename Addresses>
// inline std::size_t parse_address(It beg, It end, Addresses & addrs)
// {
//     static_assert(std::is_same<typename std::iterator_traits<It>::value_type, char>::value, "");

//     auto it = beg;
//     std::string addr, name;

//     namespace qi = boost::spirit::qi;
//     while (it != end && qi::parse(it, end, (
//             ( +(qi::char_ - '<') >> '<' >> qi::omit[*qi::space] >> +(qi::char_ - '>') >> '>' >> qi::omit[*qi::space] ) |
//             ( qi::attr(std::string()) >> qi::omit[*qi::space] >> +(qi::char_ - ',') ) ) >> -qi::lit(','), name, addr)) {
//         boost::trim_right(addr);
//         boost::trim_right(name);
//         addrs.emplace_back(addr, name);
//         addr.clear();
//         name.clear();
//     }
//     return it - beg;
// }

// template <typename Addresses>
// inline std::size_t parse_address(std::string const & text, Addresses & addrs)
// {
//     return parse_address(text.begin(), text.end(), addrs);
// }

} }
