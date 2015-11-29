/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/ethernet_arp.hpp>
#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

inline auto ethernet_arp::hardware() const -> hardware_type
{
    return static_cast<hardware_type>(ntohs(value_type::ea_hdr.ar_hrd));
}

inline void ethernet_arp::hardware(hardware_type hc)
{
    value_type::ea_hdr.ar_hrd = htons(hc);
}

inline auto ethernet_arp::protocol() const -> protocol_type
{
    return static_cast<protocol_type>(ntohs(value_type::ea_hdr.ar_pro));
}

inline void ethernet_arp::protocol(protocol_type pc)
{
    value_type::ea_hdr.ar_pro = htons(pc);
}

inline std::uint8_t ethernet_arp::hardware_length() const
{
    return value_type::ea_hdr.ar_hln;
}

inline void ethernet_arp::hardware_length(std::uint8_t len)
{
    value_type::ea_hdr.ar_hln = len;
}

inline std::uint8_t ethernet_arp::protocol_length() const
{
    return value_type::ea_hdr.ar_hln;
}

inline void ethernet_arp::protocol_length(std::uint8_t len)
{
    value_type::ea_hdr.ar_pln = len;
}

inline auto ethernet_arp::operation() const -> operation_type
{
    return static_cast<operation_type>(ntohs(value_type::ea_hdr.ar_op));
}

inline void ethernet_arp::operation(operation_type op)
{
    value_type::ea_hdr.ar_op = htons(op);
}

inline linklayer_address & ethernet_arp::sender_lladdr()
{
    auto * tmp = value_type::arp_sha;
    return *reinterpret_cast<linklayer_address *>(tmp);
}

inline linklayer_address const & ethernet_arp::sender_lladdr() const
{
    auto * tmp = value_type::arp_sha;
    return *reinterpret_cast<linklayer_address const *>(tmp);
}

inline void ethernet_arp::sender_lladdr(linklayer_address const & ll_addr)
{
    sender_lladdr() = ll_addr;
}

inline linklayer_address & ethernet_arp::target_lladdr()
{
    auto * tmp = value_type::arp_tha;
    return *reinterpret_cast<linklayer_address *>(tmp);
}

inline linklayer_address const & ethernet_arp::target_lladdr() const
{
    auto * tmp = value_type::arp_tha;
    return *reinterpret_cast<linklayer_address const *>(tmp);
}

inline void ethernet_arp::target_lladdr(linklayer_address const & ll_addr)
{
    target_lladdr() = ll_addr;
}

inline internet4_address & ethernet_arp::sender_inaddr()
{
    auto * tmp = value_type::arp_spa;
    return *reinterpret_cast<internet4_address *>(tmp);
}

inline internet4_address const & ethernet_arp::sender_inaddr() const
{
    auto * tmp = value_type::arp_spa;
    return *reinterpret_cast<internet4_address const *>(tmp);
}

inline void ethernet_arp::sender_inaddr(internet4_address const & in_addr)
{
    sender_inaddr() = in_addr;
}

inline internet4_address & ethernet_arp::target_inaddr()
{
    auto * tmp = value_type::arp_tpa;
    return *reinterpret_cast<internet4_address *>(tmp);
}

inline internet4_address const & ethernet_arp::target_inaddr() const
{
    auto * tmp = value_type::arp_tpa;
    return *reinterpret_cast<internet4_address const *>(tmp);
}

inline void ethernet_arp::target_inaddr(internet4_address const & in_addr)
{
    target_inaddr() = in_addr;
}

inline std::ostream & operator<<(std::ostream & os, ethernet_arp const & rhs)
{
    os << "arp 0x" << std::hex << static_cast<int>(rhs.operation()) << std::dec;
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
class is_match_condition<ethernet_header, ethernet_arp>
{
public:
    bool operator()(ethernet_header const & from, ethernet_arp const &) const
    {
        return from.protocol() == ethernet_header::arp;
    }
};

} } }
