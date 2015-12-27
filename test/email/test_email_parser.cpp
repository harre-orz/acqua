#include <acqua/email/email_parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <acqua/email/email.hpp>

BOOST_AUTO_TEST_SUITE(email_parser)

BOOST_AUTO_TEST_CASE(basics)
{
    acqua::email::email email;
    boost::iostreams::filtering_ostream out;
    out.push(acqua::email::email_parser(email));
    std::ifstream ifs("sample.dat");
    boost::iostreams::copy(ifs, out);
}

BOOST_AUTO_TEST_SUITE_END()
