#include <acqua/asio/pinger_v4.hpp>
#include <acqua/asio/pinger_v6.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(pinger)

BOOST_AUTO_TEST_CASE(v4)
{
    boost::asio::io_service io_service;
    acqua::asio::pinger_v4 ping(io_service);
}

BOOST_AUTO_TEST_CASE(v6)
{
    boost::asio::io_service io_service;
    acqua::asio::pinger_v6 ping(io_service);
}

BOOST_AUTO_TEST_SUITE_END()
