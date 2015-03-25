#include <acqua/asio/simple_server.hpp>
#include <iostream>

class echo_reply
    : public std::enable_shared_from_this<echo_reply>
{
public:
    typedef boost::asio::ip::tcp protocol_type;
    typedef typename protocol_type::socket socket_type;

    explicit echo_reply(boost::asio::io_service & io_service)
        : socket_(io_service)
    {
    }

    socket_type & socket()
    {
        return socket_;
    }

    void start()
    {
        boost::system::error_code ec;
        boost::asio::streambuf buf;

        std::cout << "connected from " << socket_.remote_endpoint(ec) << std::endl;
        for(;;) {
            boost::asio::read_until(socket_, buf, "\r\n");
            std::cout << "bytes-transferred " << buf.size() << std::endl;
            boost::asio::write(socket_, buf);
        }
    }

private:
    socket_type socket_;
};

int main(int, char ** argv)
{
    boost::asio::io_service io_service;
    acqua::asio::simple_server<echo_reply> sv(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), std::atoi(argv[1])));
    sv.start();
    io_service.run();
}
