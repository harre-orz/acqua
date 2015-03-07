#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/string_cast.hpp>
#include <string>

BOOST_AUTO_TEST_SUITE(string_cast)

BOOST_AUTO_TEST_CASE(string_cast)
{
    std::cout << acqua::string_cast("hello") << std::endl;
    std::cout << acqua::string_cast(L"hello") << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
