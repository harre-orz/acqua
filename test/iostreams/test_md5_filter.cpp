#include <acqua/iostreams/crypto/md5_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(md5_filter)

std::string sample_file = "sample.json";


template <typename Buf>
bool check_md5(Buf const & buf, std::string const & filename)
{
    std::ostringstream oss;
    oss << "echo "<< std::hex;
    for(auto && ch : buf)
        oss << std::setw(2) << std::setfill('0') << (static_cast<int>(ch) & 0xff);
    oss << std::dec << "  " << filename << " | md5sum -c > /dev/null";
    return (std::system(oss.str().c_str()) == 0);
}

BOOST_AUTO_TEST_CASE(input_filter)
{
    std::array<char, 16> buf;
    do {
        std::ifstream ifs(sample_file);
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::md5_filter(buf));
        in.push(ifs);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
    } while(false);
    BOOST_TEST(check_md5(buf, sample_file));
}

BOOST_AUTO_TEST_CASE(output_filter)
{
    char buf[16];
    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::md5_filter(buf));
        out.push(oss);
        std::ifstream ifs(sample_file);
        boost::iostreams::copy(ifs, out);
    } while(false);
    BOOST_TEST(check_md5(buf, sample_file));
}


BOOST_AUTO_TEST_SUITE_END()
