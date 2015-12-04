#include <acqua/asio/inotify_listener.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(inotify_listener)

struct InotifyListener : acqua::asio::inotify_listener<InotifyListener>
{
    explicit InotifyListener(boost::asio::io_service & io_service)
        : InotifyListener::base_type(io_service) {}
};

BOOST_AUTO_TEST_CASE(construct)
{
    boost::asio::io_service io_service;
    InotifyListener inotify(io_service);
}

BOOST_AUTO_TEST_SUITE_END()
