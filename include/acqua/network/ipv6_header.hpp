#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/pseudo_header.hpp>

extern "C" {
#include <netinet/in.h>
#include <netinet/ip6.h>
}

namespace acqua { namespace network {

class ipv6_header
    : public detail::header_base<ipv6_header>
    , private ::ip6_hdr
    , public detail::sourceable_and_destinable<ipv6_header, internet6_address, ::ip6_hdr, struct ::in6_addr, &::ip6_hdr::ip6_src, &ip6_hdr::ip6_dst >
{
    friend sourceable_and_destinable;
    using value_type = ::ip6_hdr;

public:
    enum protocol_type {
        tcp = 6,
        udp = 17,
        icmpv6 = 58,
    };

    template <typename It>
    void shrink_into_end(It & end) const;

    protocol_type protocol() const;

    void protocol(protocol_type n);

    // template <typename It>
    // void commit(It const & end)
    // {
    //     int len = (reinterpret_cast<std::uint8_t const *>(*&end) - reinterpret_cast<std::uint8_t const *>(this)) - sizeof(*this);
    //     value_type::ip6_ctlun.ip6_un1.ip6_un1_plen = htons(len);
    // }

    friend std::ostream & operator<<(std::ostream & os, ipv6_header const & rhs);
};

} }

#include <acqua/network/ipv6_header.ipp>
