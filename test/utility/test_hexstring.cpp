#include <acqua/utility/hexstring.hpp>
#include <boost/test/included/unit_test.hpp>
#include <algorithm>

BOOST_AUTO_TEST_SUITE(utility_hexstring);


BOOST_AUTO_TEST_CASE(basics)
{
    std::array<char, 16> buf;
    int n = {-17};
    std::generate(buf.begin(), buf.end(), [&n] { return n += 17; });

    std::ostringstream oss;
    oss << acqua::hexstring(buf);
    BOOST_TEST(oss.str() == "00112233445566778899aabbccddeeff");
}

BOOST_AUTO_TEST_SUITE_END();
