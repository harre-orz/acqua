#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/network/internet6_prefix.hpp>

BOOST_AUTO_TEST_SUITE(internet6_prefix)

BOOST_AUTO_TEST_CASE(internet6_prefix__construct)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix(internet6_address::from_string("fe80::1"), 112);
    BOOST_CHECK_EQUAL(prefix.masklen(), 112);
    BOOST_CHECK_EQUAL(prefix.address(), internet6_address::from_string("fe80::"));
    BOOST_CHECK_EQUAL(prefix.netmask(), internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:"));
    BOOST_CHECK_EQUAL(netmask_length(prefix.netmask()), 112);
}

BOOST_AUTO_TEST_CASE(internet6_prefix__incr)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix1(internet6_address::from_string("fe80::0"), 112);
    internet6_prefix prefix2(internet6_address::from_string("fe80::1:0"), 112);
    ++prefix1;
    BOOST_CHECK_EQUAL(prefix1, prefix2);
    prefix2++;
    BOOST_CHECK_EQUAL(prefix2, internet6_prefix(internet6_address::from_string("fe80::2:0"), 112));
}

BOOST_AUTO_TEST_CASE(internet6_prefix__decr)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix1(internet6_address::from_string("fe80::10:0"), 112);
    internet6_prefix prefix2(internet6_address::from_string("fe80::f:0"), 112);
    --prefix1;
    BOOST_CHECK_EQUAL(prefix1, prefix2);
    prefix2--;
    BOOST_CHECK_EQUAL(prefix2, internet6_prefix(internet6_address::from_string("fe80::e:0"), 112));
}

BOOST_AUTO_TEST_SUITE_END()
