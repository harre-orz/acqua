/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/in.h>
#include <netinet/ip6.h>
}

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/pseudo_header.hpp>

namespace acqua { namespace network {

class ipv6_header
    : private ::ip6_hdr
    , public detail::header_base<ipv6_header>
    , public detail::sourceable_and_destinable<
        ipv6_header,
        internet6_address,
        ::ip6_hdr,
        struct ::in6_addr,
        &::ip6_hdr::ip6_src,
        &ip6_hdr::ip6_dst
    >
{
    friend sourceable_and_destinable;
    using value_type = ::ip6_hdr;

public:
    typedef enum {
        tcp = 6,
        udp = 17,
        icmpv6 = 58,
    } protocol_type;

    template <typename It>
    void shrink(It & end) const
    {
        int len = (reinterpret_cast<std::uint8_t const *>(&*end) - reinterpret_cast<std::uint8_t const *>(this)) -
            (ntohs(value_type::ip6_ctlun.ip6_un1.ip6_un1_plen) + sizeof(*this));
        if (len < (reinterpret_cast<std::uint8_t const *>(&*end) - reinterpret_cast<std::uint8_t const *>(this))) {
            end -= len;
        }
    }

    protocol_type protocol() const
    {
        return static_cast<protocol_type>(value_type::ip6_nxt);
    }

    void protocol(protocol_type n)
    {
        value_type::ip6_nxt = n;
    }

    int checksum() const
    {
        return 0;
    }

    template <typename It>
    void commit(It const & end)
    {
        int len = (reinterpret_cast<std::uint8_t const *>(*&end) - reinterpret_cast<std::uint8_t const *>(this)) - sizeof(*this);
        value_type::ip6_ctlun.ip6_un1.ip6_un1_plen = htons(len);
    }

    friend std::ostream & operator<<(std::ostream & os, ipv6_header const & rhs)
    {
        os << rhs.protocol() << " source:" << rhs.source() << " destinate:" << rhs.destinate();
        return os;
    }
};


namespace detail {

template <>
class pseudo_header<ipv6_header>
{
public:
    pseudo_header(ipv6_header const * hdr, std::size_t size)
        : source_(hdr->source())
        , destinate_(hdr->destinate())
        , length_(htonl(size))
        , dummy1_(0), dummy2_(0)
        , protocol_(hdr->protocol())
    {
        static_assert(sizeof(*this) == 40, "");
        (void) dummy1_;
        (void) dummy2_;
    }

    void checksum(std::size_t & sum) const noexcept
    {
        auto const * buf = reinterpret_cast<std::uint16_t const *>(this);
        sum += buf[ 0]; sum += buf[ 1]; sum += buf[ 2]; sum += buf[ 3]; sum += buf[ 4];
        sum += buf[ 5]; sum += buf[ 6]; sum += buf[ 7]; sum += buf[ 8]; sum += buf[ 9];
        sum += buf[10]; sum += buf[11]; sum += buf[12]; sum += buf[13]; sum += buf[14];
        sum += buf[15]; sum += buf[16]; sum += buf[17]; sum += buf[18]; sum += buf[19];
    }

private:
    internet6_address source_;
    internet6_address destinate_;
    std::uint32_t length_;
    std::uint16_t dummy1_;
    std::uint8_t dummy2_;
    std::uint8_t protocol_;
} __attribute__((__packed__));

}  // detail

} }


#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <>
class is_match_condition<ethernet_header, ipv6_header>
{
public:
    bool operator()(ethernet_header const & from, ipv6_header const &) const noexcept
    {
        return from.protocol() == ethernet_header::ipv6;
    }
};

} } }
