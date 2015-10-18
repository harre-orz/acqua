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
    , public checkable<icmp_header, ::icmphdr, u_int16_t, &::icmphdr::checksum, data_checksum>
#else
    : private ::icmp
    , public checkable<icmp_header, ::icmp, u_short, &::icmp::icmp_cksum, data_checksum>
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

    message_type type() const noexcept
    {
#ifdef __linux__
        return static_cast<message_type>(value_type::type);
#else
        return static_cast<message_type>(value_type::icmp_type);
#endif
    }

    void type(message_type msg) noexcept
    {
#ifdef __linux__
        value_type::type = msg;
#else
        value_type::icmp_type = msg;
#endif
    }

    int code() const noexcept
    {
#ifdef __linux__
        return value_type::code;
#else
        return value_type::icmp_code;
#endif
    }

    void code(int n) noexcept
    {
#ifdef __linux__
        value_type::code = n;
#else
        value_type::icmp_code = n;
#endif
    }

protected:
    int id() const noexcept
    {
#ifdef __linux__
        return ntohs(value_type::un.echo.id);
#else
        return ntohs(value_type::icmp_hun.ih_idseq.icd_id);
#endif
    }

    void id(int n) noexcept
    {
#ifdef __linux__
        value_type::un.echo.id = htons(n);
#else
        value_type::icmp_hun.ih_idseq.icd_id = htons(n);
#endif
    }

    int seq() const noexcept
    {
#ifdef __linux__
        return ntohs(value_type::un.echo.sequence);
#else
        return ntohs(value_type::icmp_hun.ih_idseq.icd_seq);
#endif
    }

    void seq(int n) noexcept
    {
#ifdef __linux__
        value_type::un.echo.sequence = htons(n);
#else
        value_type::icmp_hun.ih_idseq.icd_seq = htons(n);
#endif
    }

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

#include <acqua/network/detail/impl/icmp_header.ipp>
