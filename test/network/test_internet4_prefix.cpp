#include <boost/test/included/unit_test.hpp>
#include <acqua/network/internet4_prefix.hpp>

BOOST_AUTO_TEST_SUITE(internet4_prefix)

BOOST_AUTO_TEST_CASE(construct)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix(internet4_address::from_string("192.168.100.2"), 24);
    BOOST_TEST(prefix.masklen() == 24);
    BOOST_TEST(prefix.address() == internet4_address::from_string("192.168.100.0"));
    BOOST_TEST(prefix.netmask() ==internet4_address::from_string("255.255.255.0"));
    BOOST_TEST(netmask_length(prefix.netmask()) == 24);
}

BOOST_AUTO_TEST_CASE(incr)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix1(internet4_address::from_string("192.168.100.2"), 24);
    internet4_prefix prefix2(internet4_address::from_string("192.168.101.2"), 24);
    ++prefix1;
    BOOST_TEST(prefix1 == prefix2);
    prefix2++;
    BOOST_TEST(prefix2 == internet4_prefix(internet4_address::from_string("192.168.102.0"), 24));
}

BOOST_AUTO_TEST_CASE(decr)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix1(internet4_address::from_string("192.168.100.2"), 24);
    internet4_prefix prefix2(internet4_address::from_string("192.168.99.2"), 24);
    --prefix1;
    BOOST_TEST(prefix1 == prefix2);
    prefix2--;
    BOOST_TEST(prefix2 == internet4_prefix(internet4_address::from_string("192.168.98.0"), 24));
}

BOOST_AUTO_TEST_SUITE_END()
