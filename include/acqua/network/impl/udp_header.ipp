#pragma once

#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/udp_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

inline std::ostream & operator<<(std::ostream & os, udp_header const & rhs)
{
    os << "udp "
       << " src=" << rhs.source()
       << " dst=" << rhs.destinate()
       << " check=" << std::hex << rhs.checksum() << std::dec;
    return os;
}

template <>
class is_match_condition<ipv4_header, udp_header>
{
public:
    bool operator()(ipv4_header const & from, udp_header const &) const
    {
        return from.protocol() == ipv4_header::udp;
    }
};

template <>
class is_match_condition<ipv6_header, udp_header>
{
public:
    bool operator()(ipv6_header const & from, udp_header const &) const
    {
        return from.protocol() == ipv6_header::udp;
    }
};

} } }
