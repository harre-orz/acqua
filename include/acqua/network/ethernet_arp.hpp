/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

extern "C" {
#include <netinet/if_ether.h>
}

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>

namespace acqua { namespace network { namespace detail {

/*!
  ARPクラス.
*/
class ethernet_arp
    : private ::ether_arp
    , private header_base<ethernet_arp>
{
    using base_type = header_base<ethernet_arp>;
    using value_type = ::ether_arp;

public:
    enum protocol_type {
        ip = 0x800
    };

    enum hardware_type {
        netrom = 0x00,
        ethernet = 0x01,
        experimental_ethernet = 0x02,
        ax25 = 0x03,
        pronet = 0x04,
        chaos = 0x05,
        ieee802 = 0x06,
        arcnet = 0x07,
        appletalk = 0x08,
        frame_relay = 0x15,
        atm = 0x19,
        metricom = 0x23,
        ieee1394 = 0x24,
        eui64 = 0x27,
        infiniband = 0x32,
    };

    enum operation_type {
        arp_request = 0x01,
        arp_reply   = 0x02,
        rarp_request = 0x03,
        rarp_reply   = 0x04,
        inarp_request = 0x08,
        inarp_reply = 0x09,
        arp_nak = 0x0a,
    };

    using base_type::size;
    using base_type::shrink;

    ACQUA_DECL hardware_type hardware() const
    {
        return static_cast<hardware_type>(ntohs(value_type::ea_hdr.ar_hrd));
    }

    ACQUA_DECL void hardware(hardware_type hc)
    {
        value_type::ea_hdr.ar_hrd = htons(hc);
    }

    ACQUA_DECL protocol_type protocol() const
    {
        return static_cast<protocol_type>(ntohs(value_type::ea_hdr.ar_pro));
    }

    ACQUA_DECL void protocol(protocol_type pc)
    {
        value_type::ea_hdr.ar_pro = htons(pc);
    }

    ACQUA_DECL std::uint8_t hardware_length() const
    {
        return value_type::ea_hdr.ar_hln;
    }

    ACQUA_DECL void hardware_length(std::uint8_t len)
    {
        value_type::ea_hdr.ar_hln = len;
    }

    ACQUA_DECL std::uint8_t protocol_length() const
    {
        return value_type::ea_hdr.ar_hln;
    }

    ACQUA_DECL void protocol_length(std::uint8_t len)
    {
        value_type::ea_hdr.ar_pln = len;
    }

    ACQUA_DECL operation_type operation() const
    {
        return static_cast<operation_type>(ntohs(value_type::ea_hdr.ar_op));
    }

    ACQUA_DECL void operation(operation_type ope)
    {
        value_type::ea_hdr.ar_op = htons(ope);
    }

    ACQUA_DECL linklayer_address & sender_lladdr()
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    ACQUA_DECL linklayer_address const & sender_lladdr() const
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    ACQUA_DECL void sender_lladdr(linklayer_address const & ll_addr) noexcept
    {
        sender_lladdr() = ll_addr;
    }

    ACQUA_DECL linklayer_address & target_lladdr() noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    ACQUA_DECL linklayer_address const & target_lladdr() const noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    ACQUA_DECL void target_lladdr(linklayer_address const & ll_addr) noexcept
    {
        target_lladdr() = ll_addr;
    }

    ACQUA_DECL internet4_address & sender_inaddr() noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    ACQUA_DECL internet4_address const & sender_inaddr() const noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    ACQUA_DECL void sender_inaddr(internet4_address const & in_addr) noexcept
    {
        sender_inaddr() = in_addr;
    }

    ACQUA_DECL internet4_address & target_inaddr() noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    ACQUA_DECL internet4_address const & target_inaddr() const noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    ACQUA_DECL void target_inaddr(internet4_address const & in_addr) noexcept
    {
        target_inaddr() = in_addr;
    }

    ACQUA_DECL friend std::ostream & operator<<(std::ostream & os, ethernet_arp const & rhs);
};

}  // detail

using ethernet_arp = detail::ethernet_arp;

} }

#include <acqua/network/detail/impl/ethernet_arp.ipp>
