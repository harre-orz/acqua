#include <acqua/email/detail/decode_mimeheader.hpp>
#include <boost/test/included/unit_test.hpp>
#include <map>

BOOST_AUTO_TEST_SUITE(test_decode_mimeheader)

std::string decode_mimeheader(std::string const & in)
{
    std::string out;
    acqua::email::detail::decode_mimeheader(in.begin(), in.end(), out);
    return out;
}

std::string decode_mimeheader(std::string const & in, std::map<std::string, std::string> & params)
{
    std::string out;
    params.clear();
    acqua::email::detail::decode_mimeheader(in.begin(), in.end(), out, params);
    return out;
}

std::wstring decode_mimeheader(std::wstring const & in)
{
    std::wstring out;
    acqua::email::detail::decode_mimeheader(in.begin(), in.end(), out);
    return out;
}

// std::wstring decode_mimeheader(std::wstring const & in, std::map<std::wstring, std::wstring> & params)
// {
//     std::wstring out;
//     params.clear();
//     acqua::email::detail::decode_mimeheader(in.begin(), in.end(), out, params);
//     return out;
// }

BOOST_AUTO_TEST_CASE(basics)
{
    BOOST_TEST(decode_mimeheader("subject") == "subject");
    BOOST_TEST(decode_mimeheader("hello world") == "hello world");

    BOOST_TEST((decode_mimeheader(L"subject") == L"subject"));
    BOOST_TEST((decode_mimeheader(L"hello world") == L"hello world"));
}

BOOST_AUTO_TEST_CASE(rfc2047_base64)
{
    BOOST_TEST(decode_mimeheader("=?utf-8?b?aGVsbG93b3JsZA==?=") == "helloworld");
    BOOST_TEST(decode_mimeheader("=?utf-8?b?aGVsbG8gd29ybGQ=?=") == "hello world");
    BOOST_TEST(decode_mimeheader("=?utf-8?b?aGVsbG8gIHdvcmxk=?=") == "hello  world");

    BOOST_TEST((decode_mimeheader(L"=?utf-8?b?aGVsbG93b3JsZA==?=") == L"helloworld"));
    BOOST_TEST((decode_mimeheader(L"=?utf-8?b?aGVsbG8gd29ybGQ=?=") == L"hello world"));
    BOOST_TEST((decode_mimeheader(L"=?utf-8?b?aGVsbG8gIHdvcmxk=?=") == L"hello  world"));
}

BOOST_AUTO_TEST_CASE(rfc2047_iso2022jp)
{
    BOOST_TEST(decode_mimeheader("=?iso-2022-jp?b?GyRCRnxLXDhsGyhC?=") == "日本語");
}

BOOST_AUTO_TEST_CASE(rfc2047_multiline)
{
    BOOST_TEST(decode_mimeheader("=?utf-8?b?aGVsbG8=?= =?utf-8?b?d29ybGQ=?=") == "helloworld");
}

BOOST_AUTO_TEST_CASE(dispositon_params)
{
    std::map<std::string, std::string> params;
    BOOST_TEST(decode_mimeheader("text/plain; charset=utf-8 boundary=foo", params) == "text/plain");
    BOOST_TEST(params.size() == 2);
    BOOST_TEST(params["charset"] == "utf-8");
    BOOST_TEST(params["boundary"] == "foo");
}

BOOST_AUTO_TEST_CASE(rfc2231)
{
    std::map<std::string, std::string> params;
    BOOST_TEST(decode_mimeheader(
                   "attach;"
                   " filename*='ISO-2022-JP''%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A.txt"
                   , params) == "attach");
    BOOST_TEST(params.size() == 1);
    BOOST_TEST(params["filename"] == "あいうえお.txt");
}

BOOST_AUTO_TEST_CASE(rfc2231_multiline)
{
    std::map<std::string, std::string> params;
    BOOST_TEST(decode_mimeheader(
                   "attach;"
                   " filename*1*='ISO-2022-JP''%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A"
                   " filename*2*=%E3%81%82%E3%81%84%E3%81%86%E3%81%88%E3%81%8A"
                   " filename*3*=.txt"
                   , params) == "attach");
    BOOST_TEST(params.size() == 1);
    BOOST_TEST(params["filename"] == "あいうえおあいうえお.txt");
}

BOOST_AUTO_TEST_CASE(rfc2231_msoe)
{
    std::map<std::string, std::string> params;
    BOOST_TEST(decode_mimeheader(
                   "text/plain;"
                   " name=\"=?iso-2022-jp?b?GyRCJCIkJCQmJCgkKhsoQg==?="
                   " =?iso-2022-jp?b?GyRCJCIkJCQmJCgkKhsoQg==?="
                   " =?iso-2022-jp?b?LnR4dA==?=\""
                   , params) == "text/plain");
    BOOST_TEST(params.size() == 1);
    BOOST_TEST(params["name"] == "あいうえおあいうえお.txt");
}

BOOST_AUTO_TEST_SUITE_END()
