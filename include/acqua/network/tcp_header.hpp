/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <netinet/tcp.h>
}

#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network { namespace detail {

class tcp_header
    : private ::tcphdr
    , public header_base<tcp_header>
#ifdef __linux__
    , public sourceable_and_destinable<
        tcp_header,
        std::uint16_t,
        ::tcphdr,
        ::u_int16_t,
        &::tcphdr::source,
        &::tcphdr::dest>
    , public checkable<
        tcp_header,
        ::tcphdr, u_int16_t,
        &::tcphdr::check,
        header_and_data_checksum
    >
#else  // BSD
    , public sourceable_and_destinable<
        tcp_header,
        std::uint16_t,
        ::tcphdr,
        ::u_int16_t,
        &::tcphdr::th_sport,
        &::tcphdr::th_dport
    >
    , public checkable<
        tcp_header,
        ::tcphdr,
        u_int16_t,
        &::tcphdr::th_sum,
        header_and_data_checksum
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

}  // detail

} }

#include <acqua/network/impl/tcp_header.ipp>
