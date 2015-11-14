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

namespace acqua { namespace network { namespace detail {

class icmpv6_header
    : private ::icmp6_hdr
    , public checkable<icmpv6_header, ::icmp6_hdr, u_int16_t, &icmp6_hdr::icmp6_cksum, data_checksum >
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
    , public header_base<icmpv6_echo>
{
    using base_type = icmpv6_header;

public:
    using base_type::id;
    using base_type::seq;
};


class icmpv6_neighbor
    : public icmpv6_header
    , public header_base<icmpv6_neighbor>
{
    using base_type = header_base<icmpv6_neighbor>;

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

} // detail;

using icmpv6_header = detail::icmpv6_header;
using icmpv6_echo = detail::icmpv6_echo;

} }

#include <acqua/network/impl/icmpv6_header.ipp>
