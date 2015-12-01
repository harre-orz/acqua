#pragma once

#include <boost/predef/other/endian.h>
#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

inline std::size_t ipv4_header::header_size() const
{
    return value_type::ip_hl * 4u;
}

template <typename It>
inline void ipv4_header::shrink_into_end(It & it) const
{
    auto * beg = reinterpret_cast<std::uint8_t const *>(this);
    auto * end = reinterpret_cast<std::uint8_t const *>(&*it);
    std::ptrdiff_t size = end - beg;
    std::ptrdiff_t len = size - ntohs(static_cast<std::uint16_t>(value_type::ip_len));
    if (0 < len && len < size)
        std::advance(it, -len);
}

inline auto ipv4_header::protocol() const noexcept -> protocol_type
{
    return static_cast<protocol_type>(value_type::ip_p);
}

inline void ipv4_header::protocol(protocol_type pro) noexcept
{
    value_type::ip_p = pro;
}

inline std::uint8_t ipv4_header::version() const noexcept
{
    return value_type::ip_v;
}

inline void ipv4_header::version(std::uint8_t n) noexcept
{
    value_type::ip_v = n & 0xf;
}

inline std::uint16_t ipv4_header::header_length() const noexcept
{
    return value_type::ip_hl * 4u;
}

inline void ipv4_header::header_length(std::uint16_t n) noexcept
{
    value_type::ip_hl = (n / 4) & 0x0f;
}

inline std::uint8_t ipv4_header::type_of_service() const noexcept
{
    return value_type::ip_tos;
}

inline void ipv4_header::type_of_service(std::uint8_t n) noexcept
{
    value_type::ip_tos = n;
}

inline std::uint16_t ipv4_header::total_length() const noexcept
{
    return ntohs(value_type::ip_len);
}

inline void ipv4_header::total_length(std::uint16_t n) noexcept
{
    value_type::ip_len = htons(n);
}

inline std::uint16_t ipv4_header::id() const noexcept
{
    return ntohs(value_type::ip_id);
}

inline void ipv4_header::id(std::uint16_t n) noexcept
{
    value_type::ip_id = htons(n);
}

inline bool ipv4_header::is_dont_flagment() const noexcept
{
    return ntohs(value_type::ip_off) & IP_DF;
}

inline void ipv4_header::set_dont_flagment() noexcept
{
    value_type::ip_off = htons(IP_DF);
}

inline std::uint8_t ipv4_header::time_of_live() const noexcept
{
    return value_type::ip_ttl;
}

inline void ipv4_header::time_of_live(std::uint8_t n) noexcept
{
    value_type::ip_ttl = n;
}

inline std::ostream & operator<<(std::ostream & os, ipv4_header const & rhs)
{
    os << "ipv4 " << rhs.protocol();
    switch(rhs.ip_p) {
        case ipv4_header::icmp:
            os << "(icmp)";
            break;
        case ipv4_header::tcp:
            os << "(tcp)";
            break;
        case ipv4_header::udp:
            os << "(udp)";
            break;
    }
    os << " src=" << rhs.source()
       << " dst=" << rhs.destinate()
       << " size=" << rhs.header_size()
       << " check=" << std::hex << rhs.checksum() << std::dec
       << " ver=" << rhs.version()
       << " hlen=" << rhs.header_length()
       << " tos=" << rhs.type_of_service()
       << " tlen=" << rhs.total_length()
       << " id=" << rhs.id()
       << " tol=" << rhs.time_of_live();
    return os;
}

template <>
class pseudo_header<ipv4_header>
{
public:
    explicit pseudo_header(ipv4_header const * hdr, std::size_t size)
        : sum_(0)
    {
        hdr->source().checksum(sum_);
        hdr->destinate().checksum(sum_);
        sum_ += htons(static_cast<std::uint16_t>(size));
        sum_ += static_cast<std::size_t>(hdr->protocol()
#ifdef BOOST_ENDIAN_LITTLE_BYTE
            << 8u
#endif
        );
    }

    void checksum(std::size_t & sum) const
    {
        sum = sum_;
    }

private:
    std::size_t sum_;
};


template <>
class is_match_condition<ethernet_header, ipv4_header>
{
public:
    bool operator()(ethernet_header const & from, ipv4_header const &) const
    {
        return from.protocol() == ethernet_header::ip;
    }
};

} } }
