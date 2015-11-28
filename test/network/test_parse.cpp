#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/ethernet_arp.hpp>
#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/icmp_header.hpp>
#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/icmpv6_header.hpp>
#include <acqua/network/udp_header.hpp>
#include <acqua/network/tcp_header.hpp>
#include <acqua/network/parse.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(parse)

BOOST_AUTO_TEST_CASE(ethernet_header)
{
    namespace nw = acqua::network;

    std::array<char, 1500> buf;
    auto beg = buf.begin();
    auto end = buf.end();
    auto eth = nw::parse<nw::ethernet_header>(beg, end);
    auto arp = nw::parse<nw::ethernet_arp>(eth, end);
    auto ipv4 = nw::parse<nw::ipv4_header>(eth, end);
    auto icmp = nw::parse<nw::icmp_echo>(ipv4, end);
}

BOOST_AUTO_TEST_SUITE_END()
