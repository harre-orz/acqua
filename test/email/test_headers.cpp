#include <acqua/email/headers.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(headers)

BOOST_AUTO_TEST_CASE(basics)
{
    // キーはすべて大文字・小文字の区別をしない
    acqua::email::headers headers;
    headers["Content-Type"] = "text/plain";
    BOOST_TEST((headers["content-type"] == "text/plain"));

    headers["content-type"]["charset"] = "US-ASCII";
    BOOST_TEST((headers["Content-Type"]["Charset"] == "US-ASCII"));

    headers["Content-Transfer-Encoding"] = "7bit";
    BOOST_TEST((headers["Content-Transfer-Encoding"] == "7bit"));
}

BOOST_AUTO_TEST_SUITE_END()
