#pragma once

#include <boost/predef/other/endian.h>
#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <typename It>
inline void ipv6_header::shrink_into_end(It & it) const
{
    auto * beg = reinterpret_cast<std::uint8_t const *>(this);
    auto * end = reinterpret_cast<std::uint8_t const *>(&*it);
    std::ptrdiff_t size = end - beg;
    std::ptrdiff_t len = size - static_cast<std::ptrdiff_t>(static_cast<std::size_t>(ntohs(value_type::ip6_ctlun.ip6_un1.ip6_un1_plen)) + sizeof(*this));
    if (len < size)
        std::advance(it, -len);
}

inline auto ipv6_header::protocol() const -> protocol_type
{
    return static_cast<protocol_type>(value_type::ip6_nxt);
}

inline void ipv6_header::protocol(protocol_type pro)
{
    value_type::ip6_nxt = pro;
}

inline std::ostream & operator<<(std::ostream & os, ipv6_header const & rhs)
{
    os << "ipv6 type:" << rhs.protocol()
       << " source=" << rhs.source()
       << " destinate=" << rhs.destinate();
    return os;
}

template <>
class pseudo_header<ipv6_header>
{
public:
    explicit pseudo_header(ipv6_header const * hdr, std::size_t size)
        : sum_(0)
    {
        hdr->source().checksum(sum_);
        hdr->destinate().checksum(sum_);
        std::uint32_t len = htonl(size);
        sum_ += reinterpret_cast<std::uint16_t const *>(&len)[0];
        sum_ += reinterpret_cast<std::uint16_t const *>(&len)[1];
        sum_ += static_cast<std::uint16_t>(hdr->protocol()
#ifdef BOOST_ENDIAN_LITTLE_BYTE
            << 8u
#endif
        );
    }

    void checksum(std::size_t & sum) const noexcept
    {
        sum = sum_;
    }

private:
    std::size_t sum_;
};


template <>
class is_match_condition<ethernet_header, ipv6_header>
{
public:
    bool operator()(ethernet_header const & from, ipv6_header const &) const
    {
        return from.protocol() == ethernet_header::ipv6;
    }
};

} } }
