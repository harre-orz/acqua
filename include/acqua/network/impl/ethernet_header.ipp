/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/ethernet_header.hpp>

namespace acqua { namespace network { namespace detail {

inline auto ethernet_header::protocol() const -> protocol_type
{
    return static_cast<protocol_type>( ntohs(value_type::ether_type) );
}

inline void ethernet_header::protocol(protocol_type code)
{
    value_type::ether_type = htons(code);
}

inline std::ostream & operator<<(std::ostream & os, ethernet_header const & rhs)
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
