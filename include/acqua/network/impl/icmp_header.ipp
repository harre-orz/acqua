#pragma once

#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/icmp_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

inline auto icmp_header::type() const noexcept -> message_type
{
#ifdef __linux__
    return static_cast<message_type>(value_type::type);
#else
    return static_cast<message_type>(value_type::icmp_type);
#endif
}

inline void icmp_header::type(message_type msg) noexcept
{
#ifdef __linux__
    value_type::type = msg;
#else
    value_type::icmp_type = msg;
#endif
}

inline uint icmp_header::code() const noexcept
{
#ifdef __linux__
    return value_type::code;
#else
    return value_type::icmp_code;
#endif
}

inline void icmp_header::code(uint n) noexcept
{
#ifdef __linux__
    value_type::code = static_cast<std::uint8_t>(n);
#else
    value_type::icmp_code = static_cast<std::uint8_t>(n);
#endif
}

inline uint icmp_header::id() const noexcept
{
#ifdef __linux__
    return ntohs(value_type::un.echo.id);
#else
    return ntohs(value_type::icmp_hun.ih_idseq.icd_id);
#endif
}

inline void icmp_header::id(uint n) noexcept
{
#ifdef __linux__
    value_type::un.echo.id = htons(static_cast<std::uint16_t>(n));
#else
    value_type::icmp_hun.ih_idseq.icd_id = htons(static_cast<std::uint16_t>(n));
#endif
}

inline uint icmp_header::seq() const noexcept
{
#ifdef __linux__
    return ntohs(value_type::un.echo.sequence);
#else
    return ntohs(value_type::icmp_hun.ih_idseq.icd_seq);
#endif
}

inline void icmp_header::seq(uint n) noexcept
{
#ifdef __linux__
    value_type::un.echo.sequence = htons(static_cast<std::uint16_t>(n));
#else
    value_type::icmp_hun.ih_idseq.icd_seq = htons(static_cast<std::uint16_t>(n));
#endif
}


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
