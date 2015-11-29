#include <acqua/network/interface.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(interface)

using acqua::network::interface;

BOOST_AUTO_TEST_CASE(begin_end)
{
    BOOST_TEST((interface::end() == interface::end()));
}

BOOST_AUTO_TEST_SUITE_END()
