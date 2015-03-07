#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/exception/throw_error.hpp>

BOOST_AUTO_TEST_SUITE(throw_error)

BOOST_AUTO_TEST_CASE(throw_error_construct)
{
    boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
    BOOST_CHECK_THROW(
        acqua::exception::throw_error(ec),
        boost::exception
    );

    BOOST_CHECK_THROW(
        acqua::exception::throw_error(ec),
        std::exception
    );
}

BOOST_AUTO_TEST_SUITE_END()
