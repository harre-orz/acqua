#include <acqua/asio/internet_server.hpp>
#include <boost/test/included/unit_test.hpp>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

BOOST_AUTO_TEST_SUITE(internet_server)

using protocol_type = boost::asio::ip::tcp;
using socket_type = typename protocol_type::socket;

std::mutex mutex;
std::atomic<std::size_t> count;
char buf;

struct connection : std::enable_shared_from_this<connection>
{
    using protocol_type = protocol_type;
    using lowest_layer_type = socket_type;

    socket_type socket_;

    connection(boost::asio::io_service & io_service)
        : socket_(io_service)
    {
    }

    lowest_layer_type & socket()
    {
        return socket_;
    }

    void start()
    {
    }
};

void on_receive(boost::system::error_code const & error, socket_type * socket);

void on_connect(boost::system::error_code const & error, socket_type * socket)
{
    boost::asio::detail::throw_error(error, "on_connect");
    socket->async_receive(boost::asio::buffer(&buf, 1), boost::bind(&on_receive, _1, socket));
}

void on_receive(boost::system::error_code const & error, socket_type * socket)
{
    if (error != make_error_code(boost::asio::error::eof))
        boost::asio::detail::throw_error(error, "on_receivee");

    std::size_t cnt = --count;
    boost::asio::io_service & io = socket->get_io_service();
    if (cnt >= 10) {
        socket->close();
        socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 12345),
                              boost::bind(&on_connect, _1, socket));
    } else if (cnt == 0) {
        io.stop();
    }
}

BOOST_AUTO_TEST_CASE(single)
{
    boost::asio::io_service io_service;
    acqua::asio::internet_server<connection> sv(io_service);
    sv.listen(12345);
    sv.start();

    count = 2;
    auto socket = new socket_type(io_service);
    socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 12345),
                          boost::bind(&on_connect, _1, socket));
    socket = new socket_type(io_service);
    socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v6::loopback(), 12345),
                          boost::bind(&on_connect, _1, socket));

    io_service.run();
    BOOST_TEST(count == 0);
}

BOOST_AUTO_TEST_CASE(parallels)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);
    acqua::asio::internet_server<connection> sv(io_service);
    sv.listen(12345);
    sv.start();

    boost::thread_group tg;
    for(int i = 0; i < 10; ++i)
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    count = 1000;
    for(uint i = 0; i < 5; ++i) {
        auto socket = new socket_type(io_service);
        socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 12345),
                              boost::bind(&on_connect, _1, socket));
        socket = new socket_type(io_service);
        socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v6::loopback(), 12345),
                              boost::bind(&on_connect, _1, socket));
    }

    tg.join_all();
    BOOST_TEST(count == 0);
}

BOOST_AUTO_TEST_CASE(limits)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);
    acqua::asio::internet_server<connection> sv(io_service);
    sv.max_count(10);
    sv.listen(12345);
    sv.start();

    boost::thread_group tg;
    for(int i = 0; i < 5; ++i)
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    count = 1000;
    for(uint i = 0; i < 5; ++i) {
        auto socket = new socket_type(io_service);
        socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::loopback(), 12345),
                              boost::bind(&on_connect, _1, socket));
        socket = new socket_type(io_service);
        socket->async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v6::loopback(), 12345),
                              boost::bind(&on_connect, _1, socket));
    }

    tg.join_all();
    BOOST_TEST(count == 0);
}

BOOST_AUTO_TEST_SUITE_END()
