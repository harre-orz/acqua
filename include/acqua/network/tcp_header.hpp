/*!
  The acqua library

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
        detail::header_and_data_checksum
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
        detail::header_and_data_checksum
    >
#endif
{
    friend sourceable_and_destinable;
    friend checkable;

    typedef ::tcphdr value_type;

public:
    using sourceable_and_destinable::source;  // linux の場合、変数と衝突するため

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, tcp_header const & rhs)
    {
        os << "tcp "
           << " src:" << rhs.sourceable_and_destinable::source()
           << " dst:" << rhs.sourceable_and_destinable::destinate()
           << " check:" << std::hex << rhs.checkable::checksum() << std::dec;
            ;
        return os;
    }
};


} }
