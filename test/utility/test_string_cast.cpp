#include <acqua/utility/string_cast.hpp>
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <sstream>

BOOST_AUTO_TEST_SUITE(string_cast)

BOOST_AUTO_TEST_CASE(ostream)
{
    std::ostringstream oss;
    oss << acqua::string_cast("hello");
    BOOST_TEST(oss.str() == "hello");

    oss << acqua::string_cast(L"world");
    BOOST_TEST(oss.str() == "helloworld");
}

BOOST_AUTO_TEST_CASE(string)
{
    BOOST_TEST(acqua::string_cast<std::string>("hello") == "hello");
}

BOOST_AUTO_TEST_SUITE_END()
