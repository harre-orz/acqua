#pragma once

extern "C" {
#include <netinet/if_ether.h>
}

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>

namespace acqua { namespace network {

class ethernet_arp
    : public detail::header_base<ethernet_arp>
    , private ::ether_arp
{
    typedef detail::header_base<ethernet_arp> base_type;
    typedef ::ether_arp value_type;

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

    linklayer_address & sender_hw() noexcept
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    linklayer_address const & sender_hw() const noexcept
    {
        auto * tmp = value_type::arp_sha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    void sender_hw(linklayer_address const & hw) noexcept
    {
        sender_hw() = hw;
    }

    linklayer_address & target_hw() noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address *>(tmp);
    }

    linklayer_address const & target_hw() const noexcept
    {
        auto * tmp = value_type::arp_tha;
        return *reinterpret_cast<linklayer_address const *>(tmp);
    }

    void target_hw(linklayer_address const & hw) noexcept
    {
        target_hw() = hw;
    }

    internet4_address & sender_ip() noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    internet4_address const & sender_ip() const noexcept
    {
        auto * tmp = value_type::arp_spa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    void sender_ip(internet4_address const & ip) noexcept
    {
        sender_ip() = ip;
    }

    internet4_address & target_ip() noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address *>(tmp);
    }

    internet4_address const & target_ip() const noexcept
    {
        auto * tmp = value_type::arp_tpa;
        return *reinterpret_cast<internet4_address const *>(tmp);
    }

    void target_ip(internet4_address const & ip) noexcept
    {
        target_ip() = ip;
    }

    friend std::ostream & operator<<(std::ostream & os, ethernet_arp const & rhs)
    {
        os << "arp 0x" << std::hex << rhs.protocol() << std::dec;
        os << " sdr-hw:" << rhs.sender_hw() << " tgt-hw:" << rhs.target_hw();
        os << " sdr-ip:" << rhs.sender_ip() << " tgt-ip:" << rhs.target_ip();
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
