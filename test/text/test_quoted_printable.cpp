#define BOOST_TEST_MAIN    // main関数を定義

#include <iostream>
#include <boost/test/included/unit_test.hpp>
#include <acqua/text/quoted_printable.hpp>

BOOST_AUTO_TEST_SUITE(quoted_printable)

BOOST_AUTO_TEST_CASE(quoted_printable_encode)
{

    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::encode<std::string>(std::string("12345"))), "12345");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::encode<std::string>(std::wstring(L"1_2_3.4.5"))), "1_2_3.4.5");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::encode<std::string>("123!@#")), "123=21=40=23");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::encode<std::string>("!@#$%^&*()_+|~{}:\\\\\\\"<>?[];',./=-")), "=21=40=23=24=25=5E=26=2A=28=29_=2B=7C=7E=7B=7D=3A=5C=5C=5C=22=3C=3E=3F=5B=5D=3B=27=2C.=2F=3D-");
}


BOOST_AUTO_TEST_CASE(quoted_printable_decode)
{
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::decode<std::string>(std::string("12345"))), "12345");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::decode<std::string>("1_2_3.4.5")), "1_2_3.4.5");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::decode<std::string>("123=21=40=23")), "123!@#");
    BOOST_CHECK_EQUAL((acqua::text::quoted_printable::decode<std::string>("=21=40=23=24=25=5E=26=2A=28=29_=2B=7C=7E=7B=7D=3A=5C=5C=5C=22=3C=3E=3F=5B=5D=3B=27=2C.=2F=3D-")), "!@#$%^&*()_+|~{}:\\\\\\\"<>?[];',./=-");
}

BOOST_AUTO_TEST_CASE(quoted_printable_decode_exception)
{
    BOOST_CHECK_THROW((acqua::text::quoted_printable::decode<std::string, false>("=")), std::exception);
}

BOOST_AUTO_TEST_SUITE_END()
