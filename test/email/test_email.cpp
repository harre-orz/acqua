#include <acqua/email/email.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(basics)
{
    acqua::email::email email;
    email.dump(std::cout);
}

BOOST_AUTO_TEST_SUITE_END()
