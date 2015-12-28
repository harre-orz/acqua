#include <acqua/utility/backtrace.hpp>
#include <boost/test/included/unit_test.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(utility_backtrace)

BOOST_AUTO_TEST_CASE(basics)
{
    std::ostringstream oss;
    oss << acqua::utility::backtrace;
}

// struct test_exception
//     : virtual std::exception, virtual boost::exception {};

// BOOST_AUTO_TEST_CASE(errinfo)
// {
//     test_exception ex;
//     ex << acqua::utility::errinfo_backtrace();
// }

BOOST_AUTO_TEST_SUITE_END()
