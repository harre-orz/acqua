#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/string_cast.hpp>
#include <string>
#include <sstream>

BOOST_AUTO_TEST_SUITE(string_cast)

BOOST_AUTO_TEST_CASE(string_cast_stream)
{
    std::ostringstream oss;
    oss << acqua::string_cast("hello");
    BOOST_CHECK_EQUAL(oss.str(), "hello");
    oss << acqua::string_cast(L"hello");
    BOOST_CHECK_EQUAL(oss.str(), "hellohello");
}

BOOST_AUTO_TEST_CASE(string_cast_string)
{
    BOOST_CHECK_EQUAL(acqua::string_cast<std::string>("hello"), "hello");
}

BOOST_AUTO_TEST_SUITE_END()
