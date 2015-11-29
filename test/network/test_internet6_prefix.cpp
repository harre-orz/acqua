#include <acqua/network/internet6_prefix.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(internet6_prefix)

BOOST_AUTO_TEST_CASE(construct)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix(internet6_address::from_string("fe80::1"), 112);
    BOOST_TEST(prefix.masklen() == 112);
    BOOST_TEST(prefix.address() == internet6_address::from_string("fe80::"));
    BOOST_TEST(prefix.netmask() == internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:0"));
    BOOST_TEST(netmask_length(prefix.netmask()) == 112);
}

BOOST_AUTO_TEST_CASE(incr)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix1(internet6_address::from_string("fe80::0"), 112);
    internet6_prefix prefix2(internet6_address::from_string("fe80::1:0"), 112);
    ++prefix1;
    BOOST_TEST(prefix1 == prefix2);
    prefix2++;
    BOOST_TEST(prefix2 == internet6_prefix(internet6_address::from_string("fe80::2:0"), 112));
}

BOOST_AUTO_TEST_CASE(decr)
{
    using acqua::network::internet6_address;
    using acqua::network::internet6_prefix;
    internet6_prefix prefix1(internet6_address::from_string("fe80::10:0"), 112);
    internet6_prefix prefix2(internet6_address::from_string("fe80::f:0"), 112);
    --prefix1;
    BOOST_TEST(prefix1 == prefix2);
    prefix2--;
    BOOST_TEST(prefix2 == internet6_prefix(internet6_address::from_string("fe80::e:0"), 112));
}

BOOST_AUTO_TEST_SUITE_END()
