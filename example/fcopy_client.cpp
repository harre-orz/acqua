#include <iomanip>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/iostreams/sha256_filter.hpp>


/*!
  fcopy_server に合わせて実験で作ったので、クライアントは適当
 */
std::string base64_encode(std::string const & str)
{
    std::string ret;
    do {
        boost::iostreams::filtering_ostream out;
        out.push(acqua::iostreams::base64_url_encoder());
        out.push(boost::iostreams::back_inserter(ret));
        out << str;
    } while(false);
    return ret;
}

std::string generate_fileinfo(std::string const & file)
{
    std::ostringstream oss;
    oss << "PUT"
        << " FILE=" << base64_encode(file)
        << " SIZE=" << boost::filesystem::file_size(file)
        << " PERM=000"
        << " COMPRESS=none"
        << " CHECKSUM=sha256"
        << "\r\n";
    return oss.str();
}


int main(int argc, char ** argv)
{
    std::array<unsigned char, 32> sha256sum;

    boost::asio::io_service io_service;

    boost::asio::ip::tcp::socket socket(io_service);
    socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 12345));

    boost::asio::write(socket, boost::asio::buffer(generate_fileinfo(argv[1])));

    do {
        std::ifstream ifs(argv[1]);
        boost::iostreams::filtering_istream in;
        in.push(acqua::iostreams::sha256_filter(sha256sum));
        //in.push(boost::iostreams::gzip_compressor());
        //in.push(boost::iostreams::bzip2_compressor());
        in.push(ifs);
        char buf[4096];
        std::streamsize n;
        std::size_t total = 0;
        do {
            in.read(buf, sizeof(buf));
            n = in.gcount();
            total += n;
            if (n <= 0)
                break;
            boost::asio::write(socket, boost::asio::buffer(boost::lexical_cast<std::string>(n) + "\r\n"));
            boost::asio::write(socket, boost::asio::buffer(buf, n));
        } while(true);
        std::cout << "total " << total << std::endl;
        boost::asio::write(socket, boost::asio::buffer("0\r\n", 3));
    } while(false);

    do {
        std::string checksum(sha256sum.begin(), sha256sum.end());
        checksum += "\r\n";
        boost::asio::write(socket, boost::asio::buffer(checksum));

        std::cout << std::hex;
        for(auto ch : sha256sum)
            std::cout << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
        std::cout << std::dec << std::endl;

    } while(false);

}
