#include <acqua/asio/raw.hpp>

int main(int, char ** argv)
{
    boost::asio::io_service io_service;
    acqua::asio::raw::socket socket(io_service, acqua::asio::raw::endpoint(argv[1]));
    boost::system::error_code ec;
    std::cout << ec << std::endl;
    std::cout << socket.local_endpoint(ec) << std::endl;
    std::cout << ec << std::endl;
    std::cout << socket.local_endpoint(ec).address(ec) << std::endl;
    std::cout << ec << std::endl;
}
