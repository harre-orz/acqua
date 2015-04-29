#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define private public
#define protected public
#include <acqua/network/internet4_prefix.hpp>

BOOST_AUTO_TEST_SUITE(internet4_prefix)

BOOST_AUTO_TEST_CASE(internet4_prefix__construct)
{
    using acqua::network::internet4_address;
    using acqua::network::internet4_prefix;
    internet4_prefix prefix(internet4_address::from_string("192.168.100.2"), 24);
    BOOST_CHECK_EQUAL(prefix.address(), internet4_address::from_string("192.168.100.0"));
    BOOST_CHECK_EQUAL(prefix.netmask(), internet4_address::from_string("255.255.255.0"));
}


BOOST_AUTO_TEST_SUITE_END()
