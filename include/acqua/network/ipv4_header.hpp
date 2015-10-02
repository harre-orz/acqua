/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

extern "C" {
#include <netinet/ip.h>
}

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network { namespace detail {

class ipv4_header
    : private ::ip
    , public header_base<ipv4_header>
    , public sourceable_and_destinable<
        ipv4_header,
        internet4_address,
        ::ip,
        struct in_addr,
        &::ip::ip_src,
        &::ip::ip_dst
    >
    , public checkable<
        ipv4_header,
        ::ip, u_short,
        &::ip::ip_sum,
        ipv4_checksum
    >
{
    friend sourceable_and_destinable;
    friend checkable;
    friend pseudo_header<ipv4_header>;
    using value_type = ::ip;

public:
    enum protocol_type {
        icmp = 1,
        tcp = 6,
        udp = 17,
    };

    // IPv4ヘッダーとヘッダーオプションの長さ.
    ACQUA_DECL std::size_t size() const;

    // ペイロードの終端 end を IPv4 の total_length に基づき調整する.
    template <typename It>
    ACQUA_DECL void shrink(It & end) const;

    ACQUA_DECL protocol_type protocol() const noexcept
    {
        return static_cast<protocol_type>(value_type::ip_p);
    }

    ACQUA_DECL void protocol(protocol_type n) noexcept
    {
        value_type::ip_p = n;
    }

    ACQUA_DECL int version() const noexcept
    {
        return value_type::ip_v;
    }

    ACQUA_DECL void version(int n) noexcept
    {
        value_type::ip_v = n & 0x0f;
    }

    ACQUA_DECL int header_length() const noexcept
    {
        return value_type::ip_hl * 4;
    }

    ACQUA_DECL void header_length(int n) noexcept
    {
        value_type::ip_hl = (n / 4) & 0x0f;
    }

    ACQUA_DECL int type_of_service() const noexcept
    {
        return value_type::ip_tos;
    }

    ACQUA_DECL void type_of_service(int n) noexcept
    {
        value_type::ip_tos = n;
    }

    ACQUA_DECL int total_length() const noexcept
    {
        return ntohs(value_type::ip_len);
    }

    ACQUA_DECL void total_length(int n) noexcept
    {
        value_type::ip_len = htons(n);
    }

    ACQUA_DECL int id() const noexcept
    {
        return ntohs(value_type::ip_id);
    }

    ACQUA_DECL void id(int n) noexcept
    {
        value_type::ip_id = htons(n);
    }

    ACQUA_DECL bool is_dont_flagment() const noexcept
    {
        return ntohs(value_type::ip_off) & IP_DF;
    }

    ACQUA_DECL void set_dont_flagment() noexcept
    {
        value_type::ip_off = htons(IP_DF);
    }

    ACQUA_DECL int time_of_live() const noexcept
    {
        return value_type::ip_ttl;
    }

    ACQUA_DECL void time_of_live(int n) noexcept
    {
        value_type::ip_ttl = n;
    }

    ACQUA_DECL friend std::ostream & operator<<(std::ostream & os, ipv4_header const & rhs);
};

}  // detail

using ipv4_header = detail::ipv4_header;

} }

#include <acqua/network/detail/impl/ipv4_header.ipp>
