#pragma once

#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/icmpv6_header.hpp>

namespace acqua { namespace network { namespace detail {

inline auto icmpv6_header::type() const noexcept -> message_type
{
    return static_cast<message_type>(value_type::icmp6_type);
}

inline void icmpv6_header::type(message_type msg) noexcept
{
    value_type::icmp6_type = msg;
}

inline std::uint8_t icmpv6_header::code() const noexcept
{
    return value_type::icmp6_code;
}

inline void icmpv6_header::code(std::uint8_t n) noexcept
{
    value_type::icmp6_code = n;
}

inline std::uint16_t icmpv6_header::id() const noexcept
{
    return ntohs(value_type::icmp6_dataun.icmp6_un_data16[0]);
}

inline void icmpv6_header::id(std::uint16_t n) noexcept
{
    value_type::icmp6_dataun.icmp6_un_data16[0] = htons(n);
}

inline std::uint16_t icmpv6_header::seq() const noexcept
{
    return ntohs(value_type::icmp6_dataun.icmp6_un_data16[1]);
}

inline void icmpv6_header::seq(std::uint16_t n) noexcept
{
    value_type::icmp6_dataun.icmp6_un_data16[1] = htons(n);
}

inline bool icmpv6_header::is_router() const noexcept
{
    return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x80;
}

inline void icmpv6_header::set_router(bool b) noexcept
{
    if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x80;
    else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x80;
}

inline bool icmpv6_header::is_solicited() const noexcept
{
    return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x40;
}

inline void icmpv6_header::set_solicited(bool b) noexcept
{
    if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x40;
    else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x40;
}

inline bool icmpv6_header::is_override() const noexcept
{
    return value_type::icmp6_dataun.icmp6_un_data8[0] & 0x20;
}

inline void icmpv6_header::set_override(bool b) noexcept
{
    if (b) value_type::icmp6_dataun.icmp6_un_data8[0] |=  0x20;
    else   value_type::icmp6_dataun.icmp6_un_data8[0] &= ~0x20;
}

inline internet6_address const & icmpv6_header::target_address() const noexcept
{
    return *reinterpret_cast<internet6_address const *>(value_type::icmp6_dataun.icmp6_un_data8 + 4);
}

inline void icmpv6_header::target_address(internet6_address const & addr) noexcept
{
    *reinterpret_cast<internet6_address *>(value_type::icmp6_dataun.icmp6_un_data8 + 4) = addr;
}

inline std::ostream & operator<<(std::ostream & os, icmpv6_header const & rhs)
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
