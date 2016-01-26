#include <acqua/iostreams/ostream_codecvt.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>

BOOST_AUTO_TEST_SUITE(ostream_code_converter)

BOOST_AUTO_TEST_CASE(ostream_codecvt)
{
    std::string str = "ostream_codecvt";
    std::wstring wstr = L"ostream_codecvt";

    std::ostringstream oss;
    std::wostringstream woss;
    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(oss.rdbuf()))
        << str;
    BOOST_TEST(oss.str() == str); // char => char

    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(woss.rdbuf()))
        << str;
    BOOST_TEST(woss.str() == wstr); // char => wchar_t

    oss.str("");
    woss.str(L"");
    boost::iostreams::filtering_wostream(acqua::iostreams::ostream_code_converter<wchar_t>(oss.rdbuf()))
        << wstr;
    BOOST_TEST(oss.str() == str);  // wchar_t => char

    boost::iostreams::filtering_wostream(acqua::iostreams::ostream_code_converter<wchar_t>(woss.rdbuf()))
        << wstr;
    BOOST_TEST(woss.str() == wstr);  // wchar_t => wchar_t
}


BOOST_AUTO_TEST_CASE(ostream_codecvt_japanese)
{
    std::string str = "吾輩は猫である。名前はまだ無い。";
    std::wstring wstr = L"吾輩は猫である。名前はまだ無い。";

    std::ostringstream oss;
    std::wostringstream woss;
    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(oss.rdbuf()))
        << str;
    BOOST_TEST(oss.str() == str); // char => char

    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(woss.rdbuf()))
        << str;
    BOOST_TEST(woss.str() == wstr); // char => wchar_t

    oss.str("");
    woss.str(L"");
    boost::iostreams::filtering_wostream(acqua::iostreams::ostream_code_converter<wchar_t>(oss.rdbuf()))
        << wstr;
    BOOST_TEST(oss.str() == str);  // wchar_t => char

    boost::iostreams::filtering_wostream(acqua::iostreams::ostream_code_converter<wchar_t>(woss.rdbuf()))
        << wstr;
    BOOST_TEST(woss.str() == wstr);  // wchar_t => wchar_t
}


BOOST_AUTO_TEST_CASE(ostream_codecvt_utf8_sjis)
{
    std::string str = "日本語";
    std::wstring wstr = L"日本語";
    std::string str_sjis = "\x93\xfa\x96\x7b\x8c\xea";
    std::string charset = "Shift_JIS";

    std::ostringstream oss;
    std::wostringstream woss;

    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(oss.rdbuf(), charset))
        << str_sjis;
    BOOST_TEST(oss.str() == str); // char => char

    boost::iostreams::filtering_ostream(acqua::iostreams::ostream_code_converter<char>(woss.rdbuf(), charset))
        << str_sjis;
    BOOST_TEST(woss.str() == wstr); // char => wchar_t
}


BOOST_AUTO_TEST_SUITE_END()
