#pragma once

extern "C" {
#include <arpa/inet.h>
}

#include <acqua/network/ethernet_header.hpp>

namespace acqua { namespace network { namespace detail {

std::ostream & operator<<(std::ostream & os, ethernet_header const & rhs)
{
    auto pro = rhs.protocol();
    os << "ethernet 0x" << std::hex << (int)pro << std::dec;
    switch(pro) {
        case ethernet_header::ip:
            os << "(ip)";
            break;
        case ethernet_header::arp:
            os << "(arp)";
            break;
        case ethernet_header::rarp:
            os << "(rarp)";
            break;
        case ethernet_header::ipv6:
            os << "(ipv6)";
            break;
        case ethernet_header::loopback:
            os << "(loopback)";
            break;
    }
    os << " src=" << rhs.source()
       << " dst=" << rhs.destinate();
    return os;
}

} } }
