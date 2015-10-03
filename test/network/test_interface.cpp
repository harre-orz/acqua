#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/network/interface.hpp>

BOOST_AUTO_TEST_SUITE(interface)

using acqua::network::interface;

BOOST_AUTO_TEST_CASE(interface__begin_end)
{
    BOOST_CHECK_EQUAL(interface::end() == interface::end(), true);
}

BOOST_AUTO_TEST_SUITE_END()
