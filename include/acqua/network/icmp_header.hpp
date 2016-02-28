#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/checkable.hpp>

extern "C" {
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
}

namespace acqua { namespace network {

class icmp_header
#ifdef __linux__
    : private ::icmphdr
    , public detail::checkable<icmp_header, ::icmphdr, u_int16_t, &::icmphdr::checksum, detail::payload_checksum_method>
#else
    : private ::icmp
    , public detail::checkable<icmp_header, ::icmp, u_short, &::icmp::icmp_cksum, detail::payload_checksum_method>
#endif
{
    friend checkable;

private:
#ifdef __linux__
    using value_type = ::icmphdr;
#else
    using value_type = ::icmp;
#endif

public:
    enum message_type {
        echo_reply_message = 0,
        destination_unrechable_message = 3,
        source_quench_message = 4,
        redirect_message = 5,
        echo_request_message = 8,
    };

    using checkable::checksum;  // Linux の場合、::icmphdr の checksum と衝突するため

    message_type type() const noexcept;

    void type(message_type msg) noexcept;

    std::uint8_t code() const noexcept;

    void code(std::uint8_t n) noexcept;

protected:
    ~icmp_header() = default;

    std::uint16_t id() const noexcept;

    void id(std::uint16_t n) noexcept;

    std::uint16_t seq() const noexcept;

    void seq(std::uint16_t n) noexcept;

public:
    friend std::ostream & operator<<(std::ostream & os, icmp_header const & rhs);
};


class icmp_echo
    : public icmp_header
    , public detail::header_base<icmp_echo>
{
    using base_type = header_base;

public:
    using icmp_header::id;
    using icmp_header::seq;
};

} }

#include <acqua/network/icmp_header.ipp>
