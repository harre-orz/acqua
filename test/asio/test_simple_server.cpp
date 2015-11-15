#define BOOST_TEST_MAIN
#include <acqua/asio/simple_server.hpp>
#include <mutex>
#include <thread>
#include <boost/test/included/unit_test.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

BOOST_AUTO_TEST_SUITE(simple_server)

int wait_milliseconds = 0;

struct nothing : std::enable_shared_from_this<nothing>
{
    using protocol_type = boost::asio::local::stream_protocol;
    using socket_type = typename protocol_type::socket;
    using lowest_layer_type = socket_type;

    explicit nothing(boost::asio::io_service & io_service) : socket_(io_service) {}
    lowest_layer_type & socket() { return socket_; }
    void start() { if (wait_milliseconds > 0) std::this_thread::sleep_for(std::chrono::milliseconds(wait_milliseconds)); }
    socket_type socket_;
};

std::mutex mutex;
std::atomic<std::size_t> count;
char buf;

void on_connect(boost::system::error_code const & error, boost::asio::local::stream_protocol::socket * socket);

void on_receive(boost::asio::local::stream_protocol::socket * socket)
{
    std::size_t cnt = --count;
    if (cnt >= 100) {
        socket->close();
        socket->async_connect(boost::asio::local::stream_protocol::endpoint("./test.sock"), boost::bind(&on_connect, _1, socket));
    } else {
        auto & io = socket->get_io_service();
        delete socket;
        if (cnt == 0) io.stop();
    }
}

void on_connect(boost::system::error_code const & error, boost::asio::local::stream_protocol::socket * socket)
{
    if (error) boost::asio::detail::throw_error(error, "on_connect");
    socket->async_receive(boost::asio::buffer(&buf, 1), boost::bind(&on_receive, socket));
}


BOOST_AUTO_TEST_CASE(basics)
{
    boost::asio::io_service io_service;
    acqua::asio::simple_server<nothing> sv(io_service);
    ::unlink("./test.sock");
    sv.listen("./test.sock");
    sv.start();

    count = 1;
    auto * socket = new boost::asio::local::stream_protocol::socket(io_service);
    socket->async_connect(boost::asio::local::stream_protocol::endpoint("./test.sock"), boost::bind(&on_connect, _1, socket));
    io_service.run();
}

BOOST_AUTO_TEST_CASE(parallel)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);

    boost::thread_group tg;
    for(int i = 0; i < 10; ++i)
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    acqua::asio::simple_server<nothing> sv(io_service);
    ::unlink("./test.sock");
    sv.listen("./test.sock");
    sv.start();

    count = 10000;
    for(int i = 0; i < 100; ++i) {
        auto * socket = new boost::asio::local::stream_protocol::socket(io_service);
        socket->async_connect(boost::asio::local::stream_protocol::endpoint("./test.sock"), boost::bind(&on_connect, _1, socket));
    }
    tg.join_all();
}

BOOST_AUTO_TEST_CASE(maximum)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);

    boost::thread_group tg;
    for(int i = 0; i < 10; ++i)
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    wait_milliseconds = 1;
    acqua::asio::simple_server<nothing> sv(io_service);
    ::unlink("./test.sock");
    sv.listen("./test.sock");
    sv.start();
    sv.max_count(10);

    count = 10000;
    for(int i = 0; i < 100; ++i) {
        auto * socket = new boost::asio::local::stream_protocol::socket(io_service);
        socket->async_connect(boost::asio::local::stream_protocol::endpoint("./test.sock"), boost::bind(&on_connect, _1, socket));
    }
    tg.join_all();
    io_service.stop();
}

BOOST_AUTO_TEST_SUITE_END()
