#include <acqua/iostreams/crypto/hmac_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/lexical_cast.hpp>
#include <acqua/utility/hexstring.hpp>
#include <sstream>

// See: https://en.wikipedia.org/wiki/Hash-based_message_authentication_code

BOOST_AUTO_TEST_SUITE(hmac_filter)

BOOST_AUTO_TEST_CASE(hmac_md5_input_filter)
{
    std::array<std::uint8_t, 16> buf;
    do {
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::hmac_md5_filter(buf, "", 0));
        in.push(boost::iostreams::null_source());
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "74e6f7298a9c2d168935f58c001bad88");
}

BOOST_AUTO_TEST_CASE(hmac_md5_output_filter)
{
    std::uint8_t buf[16];
    do {
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::hmac_md5_filter(buf, "key", 3));
        out.push(boost::iostreams::null_sink());
        out << "The quick brown fox jumps over the lazy dog";
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "80070713463e7749b90c2dc24911e275");
}

BOOST_AUTO_TEST_CASE(hmac_sha1_input_filter)
{
    std::array<std::uint8_t, 20> buf;
    do {
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::hmac_sha1_filter(buf, "", 0));
        in.push(boost::iostreams::null_source());
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "fbdb1d1b18aa6c08324b7d64b71fb76370690e1d");
}

BOOST_AUTO_TEST_CASE(hmac_sha1_output_filter)
{
    std::uint8_t buf[20];
    do {
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::hmac_sha1_filter(buf, "key", 3));
        out.push(boost::iostreams::null_sink());
        out << "The quick brown fox jumps over the lazy dog";
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "de7c9b85b8b78aa6bc8a7a36f70a90701c9db4d9");
}

BOOST_AUTO_TEST_CASE(hmac_sha256_input_filter)
{
    std::array<std::uint8_t, 32> buf;
    do {
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::hmac_sha256_filter(buf, "", 0));
        in.push(boost::iostreams::null_source());
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "b613679a0814d9ec772f95d778c35fc5ff1697c493715653c6c712144292c5ad");
}

BOOST_AUTO_TEST_CASE(hmac_sha256_output_filter)
{
    std::uint8_t buf[32];
    do {
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::hmac_sha256_filter(buf, "key", 3));
        out.push(boost::iostreams::null_sink());
        out << "The quick brown fox jumps over the lazy dog";
    } while(0);
    BOOST_TEST(boost::lexical_cast<std::string>(acqua::hexstring(buf)) == "f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8");
}

BOOST_AUTO_TEST_SUITE_END()
