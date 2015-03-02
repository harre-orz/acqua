/*!
  The acqua library

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

namespace acqua { namespace network {

class udp_header
    : private ::udphdr
    , public detail::header_base<udp_header>
#ifdef __linux__
    , public detail::sourceable_and_destinable<
        udp_header,
        std::uint16_t,
        ::udphdr,
        ::u_int16_t,
        &::udphdr::source,
        &::udphdr::dest
    >
    , public detail::checkable<
        udp_header,
        ::udphdr,
        u_int16_t,
        &::udphdr::check,
        detail::header_and_data_checksum
      >
#else // BSD
    , public detail::sourceable_and_destinable<
        udp_header,
        std::uint16_t,
        ::udphdr,
        ::u_int16_t,
        &::udphdr::uh_sport,
        &::udphdr::uh_dport
    >
    , public detail::checkable<
        udp_header,
        ::udphdr,
        u_int16_t,
        &::udphdr::uh_sum,
        detail::header_and_data_checksum
        >
#endif
{
    friend sourceable_and_destinable;
    friend checkable;

public:
    friend std::ostream & operator<<(std::ostream & os, udp_header const & rhs)
    {
        os << "udp "
           << " src:" << rhs.sourceable_and_destinable::source()
           << " dst:" << rhs.sourceable_and_destinable::destinate()
            ;
        return os;
    }
};

} }


#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/ipv6_header.hpp>

namespace acqua { namespace network { namespace detail {

template <>
class is_match_condition<ipv4_header, udp_header>
{
public:
    bool operator()(ipv4_header const & from, udp_header const &) const noexcept
    {
        return from.protocol() == ipv4_header::udp;
    }
};

template <>
class is_match_condition<ipv6_header, udp_header>
{
public:
    bool operator()(ipv6_header const & from, udp_header const &) const noexcept
    {
        return from.protocol() == ipv6_header::udp;
    }
};

} } }
