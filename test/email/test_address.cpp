#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/email/basic_address.hpp>
#include <acqua/email/utils/parse_address.hpp>

using namespace boost::spirit;

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(parse_address)
{
    {
        std::string str = "foo <test@example.com>";
        acqua::email::basic_address<std::string> addr;
        acqua::email::utils::address_grammar<decltype(str.begin()), ascii::space_type> g;
        BOOST_CHECK_EQUAL(qi::phrase_parse(str.begin(), str.end(), g, ascii::space, addr), true);
        BOOST_CHECK_EQUAL(addr.addrspec, "test@example.com");
        BOOST_CHECK_EQUAL(addr.namespec, "foo");
    }
    {
        std::string str = "foo@example.com";
        acqua::email::basic_address<std::string> addr;
        acqua::email::utils::address_grammar<decltype(str.begin()), ascii::space_type> g;
        BOOST_CHECK_EQUAL(qi::phrase_parse(str.begin(), str.end(), g, ascii::space, addr), true);
        BOOST_CHECK_EQUAL(addr.addrspec, "foo@example.com");
        BOOST_CHECK_EQUAL(addr.namespec, "");
    }
}

BOOST_AUTO_TEST_CASE(parse_addresses)
{
    std::string str = "foo <test@example.com>, bar <hoge@example.co.jp>";
    std::vector< acqua::email::basic_address<std::string> > addrs;
    BOOST_CHECK_EQUAL(acqua::email::utils::parse_addresses(str.begin(), str.end(), addrs), true);
}

BOOST_AUTO_TEST_SUITE_END()
