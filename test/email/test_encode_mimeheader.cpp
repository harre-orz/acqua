#include <acqua/email/encode_mimeheader.hpp>
#include <boost/test/included/unit_test.hpp>
#include <map>
BOOST_AUTO_TEST_SUITE(test_encode_mimeheader)

std::string encode_mimeheader(std::string const & key, std::string const & val, std::string const & charset = "UTF-8")
{
    std::ostringstream oss;
    acqua::email::encode_mimeheader(oss, key, val, charset);
    return oss.str();
}

std::string encode_mimeheader(std::string const & key, std::string const & val, std::map<std::string, std::string> const & params, std::string const & charset = "UTF-8")
{
    std::ostringstream oss;
    acqua::email::encode_mimeheader(oss, key, val, params, charset);
    return oss.str();
}

BOOST_AUTO_TEST_CASE(basics)
{
    BOOST_TEST(encode_mimeheader("To", "foo@example.com") == "To: foo@example.com");
    BOOST_TEST(encode_mimeheader("Cc", "foo <foo@example.com>") == "Cc: foo <foo@example.com>");
}

BOOST_AUTO_TEST_CASE(rfc2047_encode)
{
    BOOST_TEST(encode_mimeheader("Subject", "あいうえお", "utf-8") == "Subject: =?utf-8?B?44GC44GE44GG44GI44GK?=");
}

BOOST_AUTO_TEST_CASE(rfc2231_encode)
{
    std::map<std::string, std::string> params = { { "charset", "utf-8" }, { "boundary", "example" } };
    BOOST_TEST(encode_mimeheader("Content-Type", "text/plain", params) == "Content-Type: text/plain; boundary=\"example\"; charset=\"utf-8\"");
}
BOOST_AUTO_TEST_SUITE_END()
