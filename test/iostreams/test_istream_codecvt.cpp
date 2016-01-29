#include <acqua/iostreams/istream_codecvt.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>

BOOST_AUTO_TEST_SUITE(istream_code_converter)

BOOST_AUTO_TEST_CASE(istream_codevt)
{
    std::string src = "istream_codecvt";
    std::wstring wsrc = L"istream_codecvt";

    do {
        std::string dst;
        std::istringstream iss(src);
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter<char>(iss.rdbuf()))
            >> dst;
        BOOST_TEST(src == dst);  // char >> char
    } while(false);

    do {
        std::string dst;
        std::wistringstream wiss(wsrc);
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter<char>(wiss.rdbuf()))
            >> dst;
        BOOST_TEST(src == dst);  // wchar_t >> char
    } while(false);

    do {
        std::wstring wdst;
        std::istringstream iss(src);
        boost::iostreams::filtering_wistream(acqua::iostreams::istream_code_converter<wchar_t>(iss.rdbuf()))
            >> wdst;
        BOOST_TEST(wsrc == wdst);  // char >> wchar_t
    } while(false);

    do {
        std::wstring wdst;
        std::wistringstream wiss(wsrc);
        boost::iostreams::filtering_wistream(acqua::iostreams::istream_code_converter<wchar_t>(wiss.rdbuf()))
            >> wdst;
        BOOST_TEST(wsrc == wdst);  // wchar_t >> wchar_t
    } while(false);
}


BOOST_AUTO_TEST_CASE(istream_codecvt_japanese)
{
    std::string src = "吾輩は猫である。名前はまだ無い。";
    std::wstring wsrc = L"吾輩は猫である。名前はまだ無い。";

    do {
        std::string dst;
        std::istringstream iss(src);
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter<char>(iss.rdbuf()))
            >> dst;
        BOOST_TEST(src == dst);  // char >> char
    } while(false);

    do {
        std::string dst;
        std::wistringstream wiss(wsrc);
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter<char>(wiss.rdbuf()))
            >> dst;
        BOOST_TEST(src == dst);  // wchar_t >> char
    } while(false);

    do {
        std::wstring wdst;
        std::istringstream iss(src);
        boost::iostreams::filtering_wistream(acqua::iostreams::istream_code_converter<wchar_t>(iss.rdbuf()))
            >> wdst;
        BOOST_TEST(wsrc == wdst);  // char >> wchar_t
    } while(false);

    do {
        std::wstring wdst;
        std::wistringstream wiss(wsrc);
        boost::iostreams::filtering_wistream(acqua::iostreams::istream_code_converter<wchar_t>(wiss.rdbuf()))
            >> wdst;
        BOOST_TEST(wsrc == wdst);  // wchar_t >> wchar_t
    } while(false);
}


BOOST_AUTO_TEST_CASE(istream_codecvt_utf8_sjis)
{
    std::string str_sjis = "\x93\xfa\x96\x7b\x8c\xea";
    std::string charset = "Shift_JIS";

    do {
        std::string dst;
        std::istringstream iss("日本語");
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter(iss.rdbuf(), charset))
            >> dst;
        BOOST_TEST(str_sjis == dst);
    } while(false);

    do {
        std::string dst;
        std::wistringstream wiss(L"日本語");
        boost::iostreams::filtering_istream(acqua::iostreams::istream_code_converter(wiss.rdbuf(), charset))
            >> dst;
        BOOST_TEST(str_sjis == dst);
    } while(false);
}


BOOST_AUTO_TEST_CASE(istream_codecvt_utf8_sjis_multiline)
{
    std::string str_sjis = "\x93\xfa\x96\x7b\x8c\xea\n\n\x93\xfa\x96\x7b\x8c\xea";
    std::string charset = "Shift_JIS";

    do {
        std::istringstream iss("日本語\n\n日本語");
        boost::iostreams::filtering_istream in(acqua::iostreams::istream_code_converter(iss.rdbuf(), charset));
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(str_sjis == oss.str());
    } while(false);

    do {
        std::wistringstream wiss(L"日本語\n\n日本語");
        boost::iostreams::filtering_istream in(acqua::iostreams::istream_code_converter(wiss.rdbuf(), charset));
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
        BOOST_TEST(str_sjis == oss.str());
    } while(false);
}

BOOST_AUTO_TEST_SUITE_END()
