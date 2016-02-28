#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

extern "C" {
#include <netinet/tcp.h>
}

namespace acqua { namespace network {

class tcp_header
    : private ::tcphdr
    , public detail::header_base<tcp_header>
#ifdef __linux__
    , public detail::sourceable_and_destinable<
        tcp_header,
        std::uint16_t,
        ::tcphdr,
        ::u_int16_t,
        &::tcphdr::source,
        &::tcphdr::dest>
    , public detail::checkable<
        tcp_header,
        ::tcphdr, u_int16_t,
        &::tcphdr::check,
        detail::pseudo_checksum_method
    >
#else  // BSD
    , public detail::sourceable_and_destinable<
        tcp_header,
        std::uint16_t,
        ::tcphdr,
        ::u_int16_t,
        &::tcphdr::th_sport,
        &::tcphdr::th_dport
    >
    , public detail::checkable<
        tcp_header,
        ::tcphdr,
        u_int16_t,
        &::tcphdr::th_sum,
        detail::pseudo_checksum_method
    >
#endif
{
    friend sourceable_and_destinable;
    friend checkable;

    typedef ::tcphdr value_type;

public:
    using sourceable_and_destinable::source;  // linux の場合、変数と衝突するため

    friend std::ostream & operator<<(std::ostream & os, tcp_header const &);
};

} }

#include <acqua/network/tcp_header.ipp>
