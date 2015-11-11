#include <acqua/asio/simple_server.hpp>
#include <iostream>

class echo
    : public std::enable_shared_from_this<echo>
{
public:
    using protocol_type = boost::asio::ip::tcp;
    using socket_type = typename protocol_type::socket;
    using lowest_layer_type = typename socket_type::lowest_layer_type;

    explicit echo(boost::asio::io_service & io_service)
        : socket_(io_service)
    {
    }

    socket_type & socket()
    {
        return socket_;
    }

    void start()
    {
    }

private:
    socket_type socket_;
};

int main(int, char ** argv)
{
    boost::asio::io_service io_service;
    acqua::asio::simple_server<echo> sv(io_service);
    sv.listen(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::any(), atoi(argv[1])));
    sv.start();
    io_service.run();
}
