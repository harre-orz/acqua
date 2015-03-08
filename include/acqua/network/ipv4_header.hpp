/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/ip.h>
}

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network {

class ipv4_header
    : private ::ip
    , public detail::header_base<ipv4_header>
    , public detail::sourceable_and_destinable<
        ipv4_header,
        internet4_address,
        ::ip,
        struct in_addr,
        &::ip::ip_src,
        &::ip::ip_dst
    >
    , public detail::checkable<
        ipv4_header,
        ::ip, u_short,
        &::ip::ip_sum,
        detail::ipv4_checksum
    >
{
    friend sourceable_and_destinable;
    friend checkable;
    friend detail::pseudo_header<ipv4_header>;
    using value_type = ::ip;

public:
    typedef enum {
        icmp = 1,
        tcp = 6,
        udp = 17,
    } protocol_type;

    std::size_t size() const noexcept
    {
        return value_type::ip_hl * 4;
    }

    template <typename It>
    void shrink(It & end) const noexcept
    {
        int len = (reinterpret_cast<std::uint8_t const *>(&(*end)) - reinterpret_cast<std::uint8_t const *>(this)) - ntohs(value_type::ip_len);
        if (0 < len && len < (reinterpret_cast<std::uint8_t const *>(&(*end)) - reinterpret_cast<std::uint8_t const *>(this))) {
            end -= len;
        }
    }

    protocol_type protocol() const noexcept
    {
        return static_cast<protocol_type>(value_type::ip_p);
    }

    void protocol(protocol_type n) noexcept
    {
        value_type::ip_p = n;
    }

    int version() const noexcept
    {
        return value_type::ip_v;
    }

    void version(int n) noexcept
    {
        value_type::ip_v = n & 0x0f;
    }

    int header_length() const noexcept
    {
        return value_type::ip_hl;
    }

    void header_length(int n) noexcept
    {
        value_type::ip_hl = n & 0x0f;
    }

    int type_of_service() const noexcept
    {
        return value_type::ip_tos;
    }

    void type_of_service(int n) noexcept
    {
        value_type::ip_tos = n;
    }

    int total_length() const noexcept
    {
        return ntohs(value_type::ip_len);
    }

    void total_length(int n) noexcept
    {
        value_type::ip_len = htons(n);
    }

    int id() const noexcept
    {
        return ntohs(value_type::ip_id);
    }

    void id(int n) noexcept
    {
        value_type::ip_id = htons(n);
    }

    bool is_dont_flagment() const noexcept
    {
        return ntohs(value_type::ip_off) & IP_DF;
    }

    void set_dont_flagment() noexcept
    {
        value_type::ip_off = htons(IP_DF);
    }

    int time_of_live() const noexcept
    {
        return value_type::ip_ttl;
    }

    void time_of_live(int n) noexcept
    {
        value_type::ip_ttl = n;
    }

    friend std::ostream & operator<<(std::ostream & os, ipv4_header const & rhs)
    {
        os << "ipv4 " << rhs.protocol();
        switch(rhs.ip_p) {
            case icmp:
                os << "(icmp)";
                break;
            case tcp:
                os << "(tcp)";
                break;
            case udp:
                os << "(udp)";
                break;
            default:
                os << "(default)";
                break;
        }
        os << " src=" << rhs.source()
           << " dst=" << rhs.destinate()
           << " size=" << rhs.size()
           << " check=" << std::hex << rhs.checksum() << std::dec
           << " ver=" << rhs.version()
           << " hlen=" << rhs.header_length()
           << " tos=" << rhs.type_of_service()
           << " tlen=" << rhs.total_length()
           << " id=" << rhs.id()
           << " tol=" << rhs.time_of_live();
        return os;
    }
};


namespace detail {

template <>
class pseudo_header<acqua::network::ipv4_header>
{
    explicit pseudo_header(ipv4_header const * hdr, std::size_t size) noexcept
        : source_(hdr->source())
        , destinate_(hdr->destinate())
        , dummy_(0)
        , protocol_(hdr->protocol())
        , length_(htons(size))
    {
        static_assert(sizeof(*this) == 12, "");
        (void) dummy_;
    }

    void checksum(std::size_t & sum) const noexcept
    {
        auto const * buf = reinterpret_cast<std::uint16_t const *>(this);
        sum += buf[0];
        sum += buf[1];
        sum += buf[2];
        sum += buf[3];
        sum += buf[4];
        sum += buf[5];
    }

private:
    internet4_address source_;
    internet4_address destinate_;
    std::uint8_t dummy_;
    std::uint8_t protocol_;
    std::uint16_t length_;
} __attribute__((__packed__));

} } }


#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <>
class is_match_condition<ethernet_header, ipv4_header>
{
public:
    bool operator()(ethernet_header const & from, ipv4_header const &) const noexcept
    {
        return from.protocol() == ethernet_header::ip;
    }
};

} } }
