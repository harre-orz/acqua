#include <acqua/asio/netlink/netlink_listener.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(netlink_listener)

struct NetlinkLinkListener : acqua::asio::netlink::netlink_listener<NetlinkLinkListener>
{
    using category = acqua::asio::netlink::link_tag;
    NetlinkLinkListener(boost::asio::io_service & io_service)
        : NetlinkLinkListener::base_type(io_service) {}

    void on_link(std::string const, acqua::network::linklayer_address const, int, uint) {}
};

BOOST_AUTO_TEST_CASE(netlink_link)
{
    boost::asio::io_service io_service;
    NetlinkLinkListener link(io_service);
    boost::system::error_code ec;
    link.start(ec);
    link.close(ec);
}

struct NetlinkNeigh4Listener : acqua::asio::netlink::netlink_listener<NetlinkLinkListener>
{
    using category = acqua::asio::netlink::neighbor_v4_tag;
    NetlinkNeigh4Listener(boost::asio::io_service & io_service)
        : NetlinkNeigh4Listener::base_type(io_service) {}

    void on_neighbor(acqua::network::internet4_address, acqua::network::linklayer_address, uint) {}
};

BOOST_AUTO_TEST_CASE(netlink_neighbor_v4)
{
    boost::asio::io_service io_service;
    NetlinkNeigh4Listener link(io_service);
    boost::system::error_code ec;
    link.start(ec);
    link.close(ec);
}

struct NetlinkNeigh6Listener : acqua::asio::netlink::netlink_listener<NetlinkLinkListener>
{
    using category = acqua::asio::netlink::neighbor_v6_tag;
    NetlinkNeigh6Listener(boost::asio::io_service & io_service)
        : NetlinkNeigh4Listener::base_type(io_service) {}

    void on_neighbor(acqua::network::internet6_address, acqua::network::linklayer_address, uint) {}
};

BOOST_AUTO_TEST_CASE(netlink_neighbor_v6)
{
    boost::asio::io_service io_service;
    NetlinkNeigh6Listener link(io_service);
    boost::system::error_code ec;
    link.start(ec);
    link.close(ec);
}

BOOST_AUTO_TEST_SUITE_END()
