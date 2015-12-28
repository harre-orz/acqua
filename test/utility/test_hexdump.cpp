#include <acqua/utility/hexdump.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(utility_hexdump);

BOOST_AUTO_TEST_CASE(basics)
{
    std::array<char, 1024> zero;
    zero.fill(0);

    std::ostringstream oss;
    oss << acqua::hexdump(zero);
    oss << acqua::hexdump<false>(zero);
}

BOOST_AUTO_TEST_SUITE_END();
