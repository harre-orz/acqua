#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/network/internet4_prefix.hpp>

BOOST_AUTO_TEST_SUITE(internet4_prefix)

BOOST_AUTO_TEST_CASE(internet4_prefix__construct)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix(internet4_address::from_string("192.168.100.2"), 24);
    BOOST_CHECK_EQUAL(prefix.masklen(), 24);
    BOOST_CHECK_EQUAL(prefix.address(), internet4_address::from_string("192.168.100.0"));
    BOOST_CHECK_EQUAL(prefix.netmask(), internet4_address::from_string("255.255.255.0"));
    BOOST_CHECK_EQUAL(netmask_length(prefix.netmask()), 24);
}

BOOST_AUTO_TEST_CASE(internet4_prefix__incr)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix1(internet4_address::from_string("192.168.100.2"), 24);
    internet4_prefix prefix2(internet4_address::from_string("192.168.101.2"), 24);
    ++prefix1;
    BOOST_CHECK_EQUAL(prefix1, prefix2);
    prefix2++;
    BOOST_CHECK_EQUAL(prefix2, internet4_prefix(internet4_address::from_string("192.168.102.0"), 24));
}

BOOST_AUTO_TEST_CASE(internet4_prefix__decr)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix1(internet4_address::from_string("192.168.100.2"), 24);
    internet4_prefix prefix2(internet4_address::from_string("192.168.99.2"), 24);
    --prefix1;
    BOOST_CHECK_EQUAL(prefix1, prefix2);
    prefix2--;
    BOOST_CHECK_EQUAL(prefix2, internet4_prefix(internet4_address::from_string("192.168.98.0"), 24));
}


BOOST_AUTO_TEST_SUITE_END()
