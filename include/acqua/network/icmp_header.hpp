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

namespace acqua { namespace network {

class icmp_header
#ifdef __linux__
    : private ::icmphdr
    , public detail::checkable<
        icmp_header, ::icmphdr,
        u_int16_t,
        &::icmphdr::checksum,
        detail::data_checksum
    >
#else
    : private ::icmp
    , public detail::checkable<
        icmp_header,
        ::icmp,
        u_short,
        &::icmp::icmp_cksum,
        detail::data_checksum
    >
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
    typedef enum {
        echo_reply_message = 0,
        destination_unrechable_message = 3,
        source_quench_message = 4,
        redirect_message = 5,
        echo_request_message = 8,
    } message_type;

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
    friend std::ostream & operator<<(std::ostream & os, icmp_header const & rhs)
    {
        os << "icmp type:" << rhs.type();
        switch(rhs.type()) {
            case echo_reply_message:
                os << "(echo reply message)";
                os << " id:" << rhs.id();
                os << " seq:" << rhs.seq();
                break;
            case echo_request_message:
                os << "(echo request message)";
                os << " id:" << rhs.id();
                os << " seq:" << rhs.seq();
                break;
            default:
                os << "(unknown)";
                break;
        }

        return os;
    }
};


class icmp_echo
    : public icmp_header
    , public detail::header_base<icmp_echo>
{
    using base_type = detail::header_base<icmp_echo>;

public:
    using base_type::size;
    using base_type::shrink;
    using icmp_header::id;
    using icmp_header::seq;
};

} }


#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <>
class is_match_condition<ipv4_header, icmp_echo>
{
public:
    bool operator()(ipv4_header const & from, icmp_echo const & to) const noexcept
    {
        return from.protocol() == ipv4_header::icmp
            && (to.type() == icmp_header::echo_request_message || to.type() == icmp_header::echo_reply_message);
    }
};

} } }
