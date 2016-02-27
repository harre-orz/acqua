#include <acqua/utility/hexstring.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>

BOOST_AUTO_TEST_SUITE(utility_hexstring);

BOOST_AUTO_TEST_CASE(basics)
{
    std::array<char, 16> buf;
    int n = {0};
    std::generate(buf.begin(), buf.end(), [&n] { return n++; });

    std::ostringstream oss;
    oss << acqua::hexstring(buf);
    BOOST_TEST(oss.str() == "000102030405060708090a0b0c0d0e0f");
    BOOST_TEST(acqua::hexstring(buf).string() == "000102030405060708090a0b0c0d0e0f");
}

BOOST_AUTO_TEST_SUITE_END();
