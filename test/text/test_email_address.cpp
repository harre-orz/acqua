#define BOOST_TEST_MAIN    // main関数を定義

#include <acqua/text/email_address.hpp>
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(email_address)

BOOST_AUTO_TEST_CASE(parse_email_address)
{
    std::string str = "test@example.com";
    std::vector<acqua::text::email_address<std::string> > addrs;
    BOOST_CHECK_EQUAL(str.size(), acqua::text::parse_email_address(str, addrs));
    BOOST_CHECK_EQUAL(addrs[0].addrspec, "test@example.com");

    str = "foo@example.com, bar@example.com";
    BOOST_CHECK_EQUAL(str.size(), acqua::text::parse_email_address(str, addrs));
    BOOST_CHECK_EQUAL(addrs[1].addrspec, "foo@example.com");
    BOOST_CHECK_EQUAL(addrs[2].addrspec, "bar@example.com");

    str = "hello <hello@example.com>, sample@example.com,";
    BOOST_CHECK_EQUAL(str.size(), acqua::text::parse_email_address(str, addrs));
    BOOST_CHECK_EQUAL(addrs[3].addrspec, "hello@example.com");
    BOOST_CHECK_EQUAL(addrs[3].namespec, "hello");
    BOOST_CHECK_EQUAL(addrs[4].addrspec, "sample@example.com");
}

BOOST_AUTO_TEST_SUITE_END()
