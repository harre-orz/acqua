#include <acqua/asio/raw.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(raw)

BOOST_AUTO_TEST_CASE(basics)
{
    boost::system::error_code ec;
    boost::asio::io_service io_service;
    acqua::asio::raw::endpoint ep("lo", ec);
    acqua::asio::raw::socket socket(io_service);
    socket.open(acqua::asio::raw(), ec);
    socket.bind(ep, ec);
    BOOST_TEST((ep.protocol() == acqua::asio::raw()));
}

BOOST_AUTO_TEST_SUITE_END()
