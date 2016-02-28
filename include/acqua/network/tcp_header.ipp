#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/tcp_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network {

inline std::ostream & operator<<(std::ostream & os, tcp_header const & rhs)
{
    os << "tcp "
       << " src=" << rhs.source()
       << " dst=" << rhs.destinate()
       << " check=" << std::hex << rhs.checksum() << std::dec;
    return os;
}

namespace detail {

template <>
class is_match_condition<ipv4_header, tcp_header>
{
public:
    bool operator()(ipv4_header const & from, tcp_header const &) const
    {
        return from.protocol() == ipv4_header::tcp;
    }
};

template<>
class is_match_condition<ipv6_header, tcp_header>
{
public:
    bool operator()(ipv6_header const & from, tcp_header const &)
    {
        return from.protocol() == ipv6_header::tcp;
    }
};

} // detail

} }
