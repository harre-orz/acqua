#include <acqua/asio/internet_server.hpp>
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
        boost::asio::streambuf buf;
        for(;;) {
            std::size_t size = boost::asio::read(socket_, buf.prepare(10));
            std::cout << "read size:" << size << std::endl;

            buf.commit(size);
            size = boost::asio::write(socket_, buf);
            std::cout << "write size:" << size << std::endl;
        }
    }

private:
    socket_type socket_;
};

bool volatile s_marked_echo_reply;

int main(int, char ** argv)
{
    boost::asio::io_service io_service;
    acqua::asio::internet_server<echo_reply> sv(io_service, std::atoi(argv[1]), s_marked_echo_reply);
    sv.start();
    io_service.run();
}
