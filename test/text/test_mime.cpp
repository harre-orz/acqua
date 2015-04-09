#define BOOST_TEST_MAIN    // main関数を定義

#include <iostream>
#include <string>
#include <iterator>
#include <boost/test/included/unit_test.hpp>
#include <acqua/text/mime.hpp>


BOOST_AUTO_TEST_SUITE(mime)

BOOST_AUTO_TEST_CASE(decode_mimeheader_char)
{
    std::locale::global(std::locale("ja_JP.UTF-8"));
    using acqua::text::mime;

    std::string str;

    str = "sample sample sample";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == "sample sample sample");

    str = "=?iso-2022-jp?Q?=41=42ab?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == "ABab");

    str = "=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?= \r\n =?iso-2022-jp?B?eGxz?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == "本日はせいてんなり_エクセル.xls");

    str = "=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?==?iso-2022-jp?B?eGxz?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == "本日はせいてんなり_エクセル.xls");
}

/*
BOOST_AUTO_TEST_CASE(decode_mimeheader_wchar)
{
    using acqua::text::mime;

    std::wstring str;

    str = L"sample sample sample";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == L"sample sample sample");

    str = L"=?iso-2022-jp?Q?=41=42ab?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == L"ABab");

    str = L"=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?=    \r\n =?iso-2022-jp?B?eGxz?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == L"本日はせいてんなり_エクセル.xls");

    str = L"=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?==?iso-2022-jp?B?eGxz?=";
    mime::decode_mimeheader(str);
    BOOST_CHECK(str == L"本日はせいてんなり_エクセル.xls");
}
*/

BOOST_AUTO_TEST_CASE(encode_mimeheader_char)
{
    using acqua::text::mime;

    std::string str;
    str = "sample sample sample";
    mime::encode_mimeheader(str, "iso-2022b-jp");

    str = "sample sample sample";
    mime::encode_mimeheader<acqua::text::quoted_printable>(str, "iso-2022-jp");

    str = "sample sample sample";
    mime::encode_mimeheader<acqua::text::quoted_printable, acqua::text::ln_line_wrap<8> >(str, "iso-2022-jp");

    str = "sample sample sample";
    mime::encode_mimeheader<acqua::text::base64, acqua::text::ln_line_wrap<8> >(str, "utf8");
}

BOOST_AUTO_TEST_CASE(parse_param_char)
{
    using acqua::text::mime;
    std::string str;
    std::string value;
    std::map<std::string, std::string> params;

    str = "attachment;\r\n\tfilename=\"=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?=\r\n\t=?iso-2022-jp?B?eGxz?=\"";
    mime::parse_param(str, value, params);
    BOOST_CHECK_EQUAL(value, "attachment");
    BOOST_CHECK_EQUAL(params["filename"], "本日はせいてんなり_エクセル.xls");

    str = "attachment; filename*0*=iso-2022-jp''%41%42ab;\r\n filename*1*=.zip";
    mime::parse_param(str, value, params);
    BOOST_CHECK_EQUAL(value, "attachment");
    BOOST_CHECK_EQUAL(params["filename"], "ABab.zip");
}

/*
BOOST_AUTO_TEST_CASE(parse_param_wchar)
{
    using acqua::text::mime;
    std::wstring str;
    std::wstring value;
    std::map<std::wstring, std::wstring> params;

    str = L"attachment;\r\n\tfilename=\"=?iso-2022-jp?B?GyRCS1xGfCRPJDskJCRGJHMkSiRqGyhCXxskQiUoJS8lOyVrGyhCLg==?=\r\n\t=?iso-2022-jp?B?eGxz?=\"";
    mime::parse_param(str, value, params);
    BOOST_CHECK(value == L"attachment");
    BOOST_CHECK(params[L"filename"] == L"本日はせいてんなり_エクセル.xls");

    str = L"attachment; filename*0*=iso-2022-jp''%41%42ab;\r\n filename*1*=.zip";
    mime::parse_param(str, value, params);
    BOOST_CHECK(value == L"attachment");
    BOOST_CHECK(params[L"filename"] == L"ABab.zip");
}
*/

BOOST_AUTO_TEST_SUITE_END()
