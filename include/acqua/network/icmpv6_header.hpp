#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/checkable.hpp>

extern "C" {
#include <netinet/icmp6.h>
}

namespace acqua { namespace network {

class icmpv6_header
    : private ::icmp6_hdr
    , public detail::checkable<icmpv6_header, ::icmp6_hdr, u_int16_t, &icmp6_hdr::icmp6_cksum, detail::payload_checksum_method>
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

    message_type type() const noexcept;

    void type(message_type n) noexcept;

    std::uint8_t code() const noexcept;

    void code(std::uint8_t n) noexcept;

protected:
    std::uint16_t id() const noexcept;

    void id(std::uint16_t n) noexcept;

    std::uint16_t seq() const noexcept;

    void seq(std::uint16_t n) noexcept;

protected:
    bool is_router() const noexcept;

    void set_router(bool b) noexcept;

    bool is_solicited() const noexcept;

    void set_solicited(bool b) noexcept;

    bool is_override() const noexcept;

    void set_override(bool b) noexcept;

    internet6_address const & target_address() const noexcept;

    void target_address(internet6_address const & addr) noexcept;

public:
    friend std::ostream & operator<<(std::ostream & os, icmpv6_header const & rhs);
};


class icmpv6_echo
    : public icmpv6_header
    , public detail::header_base<icmpv6_echo>
{
    using base_type = header_base;

public:
    using icmpv6_header::id;
    using icmpv6_header::seq;
};


class icmpv6_neighbor
    : public icmpv6_header
    , public detail::header_base<icmpv6_neighbor>
{
    using base_type = header_base;

public:
    std::size_t header_size() const noexcept
    {
        return base_type::header_size() + sizeof(internet6_address);
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

#include <acqua/network/icmpv6_header.ipp>
