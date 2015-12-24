#include <acqua/email/message.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(message)

BOOST_AUTO_TEST_CASE(basics)
{
    acqua::email::message mes;
    std::ostream os(mes);
    os << "This is a pen.";
    BOOST_TEST(mes.str() == "This is a pen.");
    BOOST_TEST((mes.begin() != mes.end()));
    BOOST_TEST((++mes.begin() == mes.end()));
}

BOOST_AUTO_TEST_CASE(stringbuf)
{
    acqua::email::message mes;
    mes.set_payload();
}

BOOST_AUTO_TEST_CASE(filebuf)
{
    acqua::email::message mes;
    mes.set_payload("sample.txt");
}

BOOST_AUTO_TEST_SUITE_END()
