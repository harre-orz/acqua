#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define private public
#define protected public
#include <acqua/network/internet6_address.hpp>

BOOST_AUTO_TEST_SUITE(internet6_address)

BOOST_AUTO_TEST_CASE(internet6_address__construct)
{
    using acqua::network::internet6_address;

    BOOST_CHECK_EQUAL(internet6_address(), internet6_address::any());
}

BOOST_AUTO_TEST_CASE(internet6_address__increment)
{
    using acqua::network::internet6_address;

    internet6_address addr;

    ++addr;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::1"));

    addr++;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::2"));

    BOOST_CHECK_EQUAL(++internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), internet6_address());
}

BOOST_AUTO_TEST_CASE(internet6_address__decrement)
{
    using acqua::network::internet6_address;

    internet6_address addr = internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

    --addr;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"));

    addr--;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffd"));

    BOOST_CHECK_EQUAL(--internet6_address(), internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
}

BOOST_AUTO_TEST_CASE(internet6_address__add)
{
}

BOOST_AUTO_TEST_CASE(internet6_address__sub)
{
}

BOOST_AUTO_TEST_SUITE_END()
