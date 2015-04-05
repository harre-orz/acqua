#define BOOST_TEST_MAIN    // main関数を定義

#include <iostream>
#include <string>
#include <iterator>
#include <acqua/text/base64.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(base64)

BOOST_AUTO_TEST_CASE(base64_encode)
{
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>(std::string("1"))), "MQ==");
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>("12")), "MTI=");
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>("123")), "MTIz");
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>("1234")), "MTIzNA==");
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>(std::string("aiueo"))), "YWl1ZW8=");
    BOOST_CHECK_EQUAL((acqua::text::base64::encode<std::string>(std::string("!@#$%^&*()_+|~{}:;'\"<>?,."))), "IUAjJCVeJiooKV8rfH57fTo7JyI8Pj8sLg==");
    std::string a = "123", b;
    acqua::text::base64::encode(a.begin(), a.end(), std::back_insert_iterator<std::string>(b));
    BOOST_CHECK_EQUAL(b, "MTIz");
}


BOOST_AUTO_TEST_CASE(base64_decode)
{
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("MQ=="))), "1");
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("MTI="))), "12");
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("MTIz"))), "123");
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("MTIzNA=="))), "1234");
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("YWl1ZW8="))), "aiueo");
    BOOST_CHECK_EQUAL((acqua::text::base64::decode<std::string>(std::string("IUAjJCVeJiooKV8rfH57fTo7JyI8Pj8sLg=="))), "!@#$%^&*()_+|~{}:;'\"<>?,.");

    std::string a = "MTIz", b;
    acqua::text::base64::decode(a.begin(), a.end(), std::back_insert_iterator<std::string>(b));
    BOOST_CHECK_EQUAL(b, "123");

}


BOOST_AUTO_TEST_CASE(base64_decode_exception)
{
    BOOST_CHECK_THROW((acqua::text::base64::decode<std::string, false>("!")), std::exception);
    BOOST_CHECK_THROW((acqua::text::base64::decode<std::string, false>("!@")), std::exception);
    BOOST_CHECK_THROW((acqua::text::base64::decode<std::string, false>("!@#")), std::exception);
}

BOOST_AUTO_TEST_SUITE_END()
