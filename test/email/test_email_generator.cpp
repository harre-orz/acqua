#include <acqua/email/email_generator.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <acqua/email/email.hpp>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(feed_parser)
{
    acqua::email::email email;
    email["Content-Type"] = "text/plain";
    email["Content-Type"]["charset"] = "utf-8";
    std::ostream os(*email);

    os << "hello world";
    boost::iostreams::filtering_istream in;
    in.push(acqua::email::email_generator(email));
    boost::iostreams::copy(in, std::cout);
}

BOOST_AUTO_TEST_SUITE_END()
