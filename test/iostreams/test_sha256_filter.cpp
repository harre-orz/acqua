#include <acqua/iostreams/sha256_filter.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(sha256_filter)

std::string sample_file = "sample.json";


template <typename Buf>
bool check_sha256(Buf const & buf, std::string const & filename)
{
    std::ostringstream oss;
    oss << "echo "<< std::hex;
    for(auto && ch : buf)
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
    oss << std::dec << "  " << filename << " | sha256sum -c > /dev/null";
    return (std::system(oss.str().c_str()) == 0);
}

BOOST_AUTO_TEST_CASE(input_filter)
{
    acqua::iostreams::sha256_filter::buffer_type buf;
    do {
        std::ifstream ifs(sample_file);
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::sha256_filter(buf));
        in.push(ifs);
        std::ostringstream oss;
        boost::iostreams::copy(in, oss);
    } while(false);
    BOOST_TEST(check_sha256(buf, sample_file));
}

BOOST_AUTO_TEST_CASE(output_filter)
{
    acqua::iostreams::sha256_filter::buffer_type buf;
    do {
        std::ostringstream oss;
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::sha256_filter(buf));
        out.push(oss);
        std::ifstream ifs(sample_file);
        boost::iostreams::copy(ifs, out);
    } while(false);
    BOOST_TEST(check_sha256(buf, sample_file));
}


BOOST_AUTO_TEST_SUITE_END()
