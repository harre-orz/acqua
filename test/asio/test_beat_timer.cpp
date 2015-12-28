#include <acqua/asio/beat_timer.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/asio/steady_timer.hpp>

BOOST_AUTO_TEST_SUITE(beat_timer);

static boost::system::error_code result_code;

class BeatTimer
    : public acqua::asio::beat_timer<BeatTimer, boost::asio::steady_timer>
{
private:
public:
    explicit BeatTimer(boost::asio::io_service & io_service)
        : BeatTimer::base_type(io_service) {}

    void on_run(boost::system::error_code const & error)
    {
        //std::cout << error.message() << std::endl;
        BOOST_TEST(error == result_code);
    }

    void on_run(boost::system::error_code const &, int num)
    {
        BOOST_TEST(num == 100);
    }
};


BOOST_AUTO_TEST_CASE(basics)
{
    boost::asio::io_service io_service;
    BeatTimer beat(io_service);
    result_code.clear();  // Success
    beat.async_wait(std::chrono::steady_clock::now());
    io_service.run();
}

BOOST_AUTO_TEST_CASE(cancel)
{
    boost::asio::io_service io_service;
    BeatTimer beat(io_service);
    result_code = boost::system::error_code(boost::system::errc::operation_canceled, boost::system::system_category());
    beat.async_wait(std::chrono::steady_clock::now() + std::chrono::seconds(1));
    beat.cancel();
    io_service.run();
}

BOOST_AUTO_TEST_CASE(args)
{
    boost::asio::io_service io_service;
    BeatTimer beat(io_service);
    result_code.clear();
    int n = 100;
    beat.async_wait(std::chrono::steady_clock::now(), n);
    io_service.run();
}

BOOST_AUTO_TEST_SUITE_END();
