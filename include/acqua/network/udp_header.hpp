/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/udp.h>
}

#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network { namespace detail {

class udp_header
    : private ::udphdr
    , public header_base<udp_header>
#ifdef __linux__
    , public sourceable_and_destinable<
        udp_header,
        std::uint16_t,
        ::udphdr,
        ::u_int16_t,
        &::udphdr::source,
        &::udphdr::dest
    >
    , public checkable<
        udp_header,
        ::udphdr,
        u_int16_t,
        &::udphdr::check,
        header_and_data_checksum
      >
#else // BSD
    , public sourceable_and_destinable<
        udp_header,
        std::uint16_t,
        ::udphdr,
        ::u_int16_t,
        &::udphdr::uh_sport,
        &::udphdr::uh_dport
    >
    , public checkable<
        udp_header,
        ::udphdr,
        u_int16_t,
        &::udphdr::uh_sum,
        header_and_data_checksum
        >
#endif
{
    friend sourceable_and_destinable;
    friend checkable;

public:
    friend std::ostream & operator<<(std::ostream & os, udp_header const & rhs);
    using sourceable_and_destinable::source;
};

}  // detail

} }

#include <acqua/network/impl/udp_header.ipp>
