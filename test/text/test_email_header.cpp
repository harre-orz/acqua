#define BOOST_TEST_MAIN    // main関数を定義

#include <acqua/text/email_header.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(email_header)

BOOST_AUTO_TEST_CASE(email_header_basics)
{
    acqua::text::email_header<std::string> header;

    BOOST_CHECK_EQUAL(header["Content-Type"], "");

    header["Content-Type"] = "text/plain";
    header["Content-Type"]["charset"] = "UTF-8";
    header["content-type"]["boundary"] = "__boundary__";
    header["content-transfer-encoding"] = "bas64";
    BOOST_CHECK_EQUAL(header["content-type"], "text/plain");
    BOOST_CHECK_EQUAL(header["content-type"]["Charset"], "UTF-8");
    BOOST_CHECK_EQUAL(header["CONTENT-TYPE"]["BOUNDARY"], "__boundary__");
    // BOOST_CHECK_EQUAL(header.size(), 2);
    // BOOST_CHECK_EQUAL(header.size("content-type"), 3);
    // BOOST_CHECK_EQUAL(header.size("content-transfer-encoding"), 1);
    // BOOST_CHECK_EQUAL(header.size("hogehoge"), 0);
    // BOOST_CHECK_EQUAL(header.empty("content-type"), false);
    // BOOST_CHECK_EQUAL(header.empty("hogehoge"), true);
    // header.clear("content-type");
    // BOOST_CHECK_EQUAL(header.empty("content-type"), true);
    // BOOST_CHECK_EQUAL(header.empty("hogehoge"), true);
    header.dump(std::cout);
}

BOOST_AUTO_TEST_SUITE_END()
