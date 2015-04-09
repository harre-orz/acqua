/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/if_ether.h>
}

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>

namespace acqua { namespace network {

/*!
  ARPクラス
*/
class ethernet_arp
    : private ::ether_arp
    , public detail::header_base<ethernet_arp>
{
    using base_type = detail::header_base<ethernet_arp>;
    using value_type = ::ether_arp;

public:
    typedef enum {
        ip = 0x800
    } protocol_code;

    typedef enum {
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
    } hardware_code;

    typedef enum {
        arp_request = 0x01,
        arp_reply   = 0x02,
        rarp_request = 0x03,
        rarp_reply   = 0x04,
        inarp_request = 0x08,
        inarp_reply = 0x09,
        arp_nak = 0x0a,
    } operation_code;

    using base_type::size;
    using base_type::shrink;

    hardware_code hardware() const noexcept
    {
        return static_cast<hardware_code>(ntohs(value_type::ea_hdr.ar_hrd));
    }

    void hardware(hardware_code hc) noexcept
    {
        value_type::ea_hdr.ar_hrd = htons(hc);
    }

    protocol_code protocol() const noexcept
    {
        return static_cast<protocol_code>(ntohs(value_type::ea_hdr.ar_pro));
    }

    void protocol(protocol_code pc) noexcept
    {
        value_type::ea_hdr.ar_pro = htons(pc);
    }

    std::uint8_t hardware_length() const noexcept
    {
        return value_type::ea_hdr.ar_hln;
    }

    void hardware_length(std::uint8_t len) noexcept
    {
        value_type::ea_hdr.ar_hln = len;
    }

    std::uint8_t protocol_length() const noexcept
    {
        return value_type::ea_hdr.ar_hln;
    }

    void protocol_length(std::uint8_t len) noexcept
    {
        value_type::ea_hdr.ar_pln = len;
    }

    operation_code operation() const noexcept
    {
        return static_cast<operation_code>(ntohs(value_type::ea_hdr.ar_op));
    }

    void operation(operation_code code) noexcept
    {
        value_type::ea_hdr.ar_op = htons(code);
    }

    linklayer_address & sender_ll() noexcept
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    linklayer_address const & sender_ll() const noexcept
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    void sender_ll(linklayer_address const & ll_addr) noexcept
    {
        sender_ll() = ll_addr;
    }

    linklayer_address & target_ll() noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    linklayer_address const & target_ll() const noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    void target_ll(linklayer_address const & ll_addr) noexcept
    {
        target_ll() = ll_addr;
    }

    internet4_address & sender_in() noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    internet4_address const & sender_in() const noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    void sender_in(internet4_address const & in_addr) noexcept
    {
        sender_in() = in_addr;
    }

    internet4_address & target_in() noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    internet4_address const & target_in() const noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    void target_in(internet4_address const & in_addr) noexcept
    {
        target_in() = in_addr;
    }

    friend std::ostream & operator<<(std::ostream & os, ethernet_arp const & rhs)
    {
        os << "arp 0x" << std::hex << rhs.operation() << std::dec;
        switch(rhs.operation()) {
            case arp_request:
                os << "(arp request)";
                break;
            case arp_reply:
                os << "(arp reply)";
                break;
            case rarp_request:
                os << "(rarp request)";
                break;
            case rarp_reply:
                os << "(rarp reply)";
                break;
            case inarp_request:
                os << "(inarp request)";
                break;
            case inarp_reply:
                os << "(inarp reply)";
                break;
            case arp_nak:
                os << "(arp nak)";
                break;
        }
        os << " sdr-ll:" << rhs.sender_ll() << " tgt-ll:" << rhs.target_ll();
        os << " sdr-in:" << rhs.sender_in() << " tgt-in:" << rhs.target_in();
        return os;
    }
};

} }


#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <>
class is_match_condition<ethernet_header, ethernet_arp>
{
public:
    bool operator()(ethernet_header const & from, ethernet_arp const &) const noexcept
    {
        return from.protocol() == ethernet_header::arp;
    }
};

} } }
