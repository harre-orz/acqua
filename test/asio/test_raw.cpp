#include <acqua/asio/raw.hpp>

int main(int, char ** argv)
{
    boost::asio::io_service io_service;
    acqua::asio::raw::socket socket(io_service, acqua::asio::raw::endpoint(argv[1]));
}
