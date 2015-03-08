/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <arpa/inet.h>
#include <net/ethernet.h>
}

#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/is_match_condition.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>

namespace acqua { namespace network {

/*!
  イーサネットヘッダークラス.
*/
class ethernet_header
    : private ::ether_header
    , public detail::header_base<ethernet_header>
#ifdef __linux__
    , public detail::sourceable_and_destinable<
        ethernet_header,
        linklayer_address,
        ::ether_header,
        u_int8_t[ETH_ALEN],
        &::ether_header::ether_shost,
        &::ether_header::ether_dhost
    >
#else
    , public detail::sourceable_and_destinable<
        ethernet_header,
        linklayer_address,
        ::ether_header,
        u_int8_t[ETHER_ADDR_LEN],
        &::ether_header::ether_shost,
        &::ether_header::ether_dhost
    >
#endif
{
    friend sourceable_and_destinable;
    using base_type = detail::header_base<ethernet_header>;
    using value_type = ::ether_header;

public:
    typedef enum {
        ip = 0x0800,
        arp = 0x0806,
        rarp = 0x8035,
        ipv6 = 0x86dd,
        loopback = 0x9000,
    } protocol_code;

    using base_type::size;
    using base_type::shrink;

    protocol_code protocol() const noexcept
    {
        return static_cast<protocol_code>(ntohs(value_type::ether_type));
    }

    void protocol(protocol_code code) noexcept
    {
        value_type::ether_type = htons(code);
    }

    friend std::ostream & operator<<(std::ostream & os, ethernet_header const & rhs)
    {
        auto pro = rhs.protocol();
        os << "ethernet 0x" << std::hex << pro << std::dec;
        switch(pro) {
            case ip:
                os << "(ip)";
                break;
            case arp:
                os << "(arp)";
                break;
            case rarp:
                os << "(rarp)";
                break;
            case ipv6:
                os << "(ipv6)";
                break;
            default:
                os << "(unknown)";
                break;
        }
        os << " src=" << rhs.source()
           << " dst=" << rhs.destinate();
        return os;
    }
};

} }
