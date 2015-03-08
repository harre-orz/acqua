#include <acqua/asio/pinger.hpp>

int main(int, char **)
{
    boost::asio::io_service io_service;
    acqua::asio::pinger_v4 p(io_service, 0);
    p.start();
    p.ping("localhost", std::chrono::seconds(10) , [](boost::system::error_code const & error, boost::asio::ip::icmp::endpoint const & ep) {
        std::cout << error << ' ' << error.message() << std::endl;
});
    io_service.run();

}
