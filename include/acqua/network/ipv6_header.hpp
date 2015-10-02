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

namespace acqua { namespace network { namespace detail {

class ipv6_header
    : private ::ip6_hdr
    , public header_base<ipv6_header>
    , public sourceable_and_destinable<
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
    enum protocol_type {
        tcp = 6,
        udp = 17,
        icmpv6 = 58,
    };

    template <typename It>
    ACQUA_DECL void shrink(It & end) const;

    ACQUA_DECL protocol_type protocol() const
    {
        return static_cast<protocol_type>(value_type::ip6_nxt);
    }

    ACQUA_DECL void protocol(protocol_type n)
    {
        value_type::ip6_nxt = n;
    }

    ACQUA_DECL int checksum() const
    {
        return 0;
    }

    template <typename It>
    ACQUA_DECL void commit(It const & end);

    ACQUA_DECL friend std::ostream & operator<<(std::ostream & os, ipv6_header const & rhs);
};

}  // detail

using ipv6_header = detail::ipv6_header;

} }

#include <acqua/network/detail/impl/ipv6_header.ipp>
