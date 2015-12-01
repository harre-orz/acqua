/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

extern "C" {
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
}

#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network { namespace detail {

class icmp_header
#ifdef __linux__
    : private ::icmphdr
    , public checkable<icmp_header, ::icmphdr, u_int16_t, &::icmphdr::checksum, payload_checksum_method>
#else
    : private ::icmp
    , public checkable<icmp_header, ::icmp, u_short, &::icmp::icmp_cksum, payload_checksum_method>
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

    uint code() const noexcept;

    void code(uint n) noexcept;

protected:
    ~icmp_header() = default;

    uint id() const noexcept;

    void id(uint n) noexcept;

    uint seq() const noexcept;

    void seq(uint n) noexcept;

public:
    friend std::ostream & operator<<(std::ostream & os, icmp_header const & rhs);
};


class icmp_echo
    : public icmp_header
    , public header_base<icmp_echo>
{
    using base_type = header_base<icmp_echo>;

public:
    using icmp_header::id;
    using icmp_header::seq;
};

}  // detail

using icmp_echo = detail::icmp_echo;

} }

#include <acqua/network/impl/icmp_header.ipp>
