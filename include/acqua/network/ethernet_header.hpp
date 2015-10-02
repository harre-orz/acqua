/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

extern "C" {
#include <net/ethernet.h>
}

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>

namespace acqua { namespace network { namespace detail {

/*!
  イーサネットヘッダークラス.
*/
class ethernet_header
    : private ::ether_header
    , private header_base<ethernet_header>
#ifdef __linux__
    , public sourceable_and_destinable<
        ethernet_header,
        linklayer_address,
        ::ether_header,
        u_int8_t[ETH_ALEN],
        &::ether_header::ether_shost,
        &::ether_header::ether_dhost
    >
#else
    , public sourceable_and_destinable<
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
    using base_type = header_base<ethernet_header>;
    using value_type = ::ether_header;

public:
    enum protocol_type {
        ip = 0x0800,
        arp = 0x0806,
        rarp = 0x8035,
        ipv6 = 0x86dd,
        loopback = 0x9000,
    };

    using base_type::size;
    using base_type::shrink;

    ACQUA_DECL protocol_type protocol() const
    {
        return static_cast<protocol_type>( ntohs(value_type::ether_type) );
    }

    ACQUA_DECL void protocol(protocol_type code)
    {
        value_type::ether_type = htons(code);
    }

    ACQUA_DECL friend std::ostream & operator<<(std::ostream & os, ethernet_header const & rhs);
};

}  // detail

using ethernet_header = detail::ethernet_header;

} }

#include <acqua/network/detail/impl/ethernet_header.ipp>
