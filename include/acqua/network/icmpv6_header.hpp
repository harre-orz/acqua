/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/icmp6.h>
}

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network {

class icmpv6_header
    : private ::icmp6_hdr
    , public detail::checkable<
        icmpv6_header,
        ::icmp6_hdr,
        u_int16_t,
        &icmp6_hdr::icmp6_cksum,
        detail::data_checksum
    >
{
    friend checkable;
    using value_type = ::icmp6_hdr;

public:
    enum message_type {
        echo_request_message = 128,
        echo_reply_message   = 129,
        neighbor_solicitation_message = 135,
        neighbor_advertisement_message = 136,
    };

    message_type type() const noexcept
    {
        return static_cast<message_type>(value_type::icmp6_type);
    }

    void type(message_type n) noexcept
    {
        value_type::icmp6_type = n;
    }

    std::uint8_t code() const noexcept
    {
        return value_type::icmp6_code;
    }

    void code(std::uint8_t n) noexcept
    {
        value_type::icmp6_code = n;
    }

protected:
    std::uint16_t id() const noexcept
    {
        return ntohs(value_type::icmp6_dataun.icmp6_un_data16[0]);
    }

    void id(std::uint16_t n) noexcept
    {
        value_type::icmp6_dataun.icmp6_un_data16[0] = htons(n);
    }

    std::uint16_t seq() const noexcept
    {
        return ntohs(value_type::icmp6_dataun.icmp6_un_data16[1]);
    }

    void seq(std::uint16_t n) noexcept
    {
        value_type::icmp6_dataun.icmp6_un_data16[1] = htons(n);
    }

protected:
    bool is_router() const noexcept
    {
        return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x80;
    }

    void is_router(bool b) noexcept
    {
        if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x80;
        else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x80;
    }

    bool is_solicited() const noexcept
    {
        return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x40;
    }

    void is_solicited(bool b) noexcept
    {
        if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x40;
        else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x40;
    }

    bool is_override() const noexcept
    {
        return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x20;
    }

    void is_override(bool b) noexcept
    {
        if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x20;
        else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x20;
    }

    internet6_address const & target_address() const noexcept
    {
        return *reinterpret_cast<internet6_address const *>(value_type::icmp6_dataun.icmp6_un_data8 + 4);
    }

    void target_address(internet6_address const & addr) noexcept
    {
        *reinterpret_cast<internet6_address *>(value_type::icmp6_dataun.icmp6_un_data8 + 4) = addr;
    }

public:
    friend std::ostream & operator<<(std::ostream & os, icmpv6_header const & rhs);
};


class icmpv6_echo
    : public icmpv6_header
    , public detail::header_base<icmpv6_echo>
{
    using base_type = icmpv6_header;

public:
    using base_type::id;
    using base_type::seq;
};


class icmpv6_neighbor
    : public icmpv6_header
    , public detail::header_base<icmpv6_neighbor>
{
    using base_type = detail::header_base<icmpv6_neighbor>;

public:
    std::size_t size() const noexcept
    {
        return base_type::size() + sizeof(internet6_address);
    }

    using icmpv6_header::is_router;
    using icmpv6_header::is_solicited;
    using icmpv6_header::is_override;
    using icmpv6_header::target_address;

    typedef enum {
        source_linklayer_address = 1,
        target_linklayer_address = 2,
        prefix_information = 3,
        redirected_header = 4,
        mtu = 5,
    } message_type;

    class iterator;
    class const_iterator;
};

} }
