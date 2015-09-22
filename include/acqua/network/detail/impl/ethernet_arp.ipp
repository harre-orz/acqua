#pragma once

#include <acqua/network/ethernet_arp.hpp>
#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

std::ostream & operator<<(std::ostream & os, ethernet_arp const & rhs)
{
    os << "arp 0x" << std::hex << rhs.operation() << std::dec;
    switch(rhs.operation()) {
        case ethernet_arp::arp_request:
            os << "(arp request)";
            break;
        case ethernet_arp::arp_reply:
            os << "(arp reply)";
            break;
        case ethernet_arp::rarp_request:
            os << "(rarp request)";
            break;
        case ethernet_arp::rarp_reply:
            os << "(rarp reply)";
            break;
        case ethernet_arp::inarp_request:
            os << "(inarp request)";
            break;
        case ethernet_arp::inarp_reply:
            os << "(inarp reply)";
            break;
        case ethernet_arp::arp_nak:
            os << "(arp nak)";
            break;
    }
    os << " sdr-ll=" << rhs.sender_lladdr() << " tgt-ll=" << rhs.target_lladdr()
       << " sdr-in=" << rhs.sender_inaddr() << " tgt-in=" << rhs.target_inaddr();
    return os;
}

template <>
struct is_match_condition<ethernet_header, ethernet_arp>
{
    bool operator()(ethernet_header const & from, ethernet_arp const &) const
    {
        return from.protocol() == ethernet_header::arp;
    }
};

} } }
