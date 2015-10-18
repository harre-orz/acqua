#pragma once

#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/icmp_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

inline std::ostream & operator<<(std::ostream & os, icmp_header const & rhs)
{
    os << "icmp type:" << rhs.type();
    switch(rhs.type()) {
        case icmp_header::echo_request_message:
            os << "(echo request message)"
               << " id=" << rhs.id()
               << " seq=" << rhs.seq();
            break;
        case icmp_header::echo_reply_message:
            os << "(echo reply message)"
               << " id=" << rhs.id()
               << " seq=" << rhs.seq();
            break;
        case icmp_header::destination_unrechable_message:
            os << "(destination unreachable message)";
            break;
        case icmp_header::source_quench_message:
            os << "(source quench message)";
            break;
        case icmp_header::redirect_message:
            os << "(redirect message)";
            break;
    }
    return os;
}

template <>
class is_match_condition<ipv4_header, icmp_echo>
{
public:
    bool operator()(ipv4_header const & from, icmp_echo const & to) const
    {
        return from.protocol() == ipv4_header::icmp
            && (to.type() == icmp_header::echo_request_message || to.type() == icmp_header::echo_reply_message);
    }
};

} } }
