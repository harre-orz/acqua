#pragma once

#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/icmpv6_header.hpp>

namespace acqua { namespace network { namespace detail {

std::ostream & operator<<(std::ostream & os, icmpv6_header const & rhs)
{
    os << "icmpv6 " << rhs.type();
    return os;
}

template <>
class is_match_condition<ipv6_header, icmpv6_echo>
{
public:
    bool operator()(ipv6_header const & from, icmpv6_echo const & to) const
    {
        return from.protocol() == ipv6_header::icmpv6
            && (to.type() == icmpv6_neighbor::echo_request_message
                || to.type() == icmpv6_header::echo_reply_message);
    }
};


template <>
class is_match_condition<ipv6_header, icmpv6_neighbor>
{
public:
    bool operator()(ipv6_header const & from, icmpv6_neighbor const & to)
    {
        return from.protocol() == ipv6_header::icmpv6
            && (to.type() == icmpv6_neighbor::neighbor_solicitation_message
                || to.type() == icmpv6_neighbor::neighbor_advertisement_message);
    }
};

} } }
