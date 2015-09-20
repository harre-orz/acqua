#pragma once

#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/ipv6_header.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network {

template <typename It>
void ipv6_header::shrink(It & end) const
{
    int len = (reinterpret_cast<std::uint8_t const *>(&*end) - reinterpret_cast<std::uint8_t const *>(this)) -
        (ntohs(value_type::ip6_ctlun.ip6_un1.ip6_un1_plen) + sizeof(*this));
    if (len < (reinterpret_cast<std::uint8_t const *>(&*end) - reinterpret_cast<std::uint8_t const *>(this))) {
        end -= len;
    }
}

template <typename It>
void ipv6_header::commit(It const & end)
{
    int len = (reinterpret_cast<std::uint8_t const *>(*&end) - reinterpret_cast<std::uint8_t const *>(this)) - sizeof(*this);
    value_type::ip6_ctlun.ip6_un1.ip6_un1_plen = htons(len);
}


std::ostream & operator<<(std::ostream & os, ipv6_header const & rhs)
{
    os << "ipv6 type:" << rhs.protocol()
       << " source=" << rhs.source()
       << " destinate=" << rhs.destinate();
    return os;
}

namespace detail {

template <>
class pseudo_header<ipv6_header>
{
public:
    explicit pseudo_header(ipv6_header const * hdr, std::size_t size)
        : source_(hdr->source())
        , destinate_(hdr->destinate())
        , length_(htonl(size))
        , dummy1_(0), dummy2_(0)
        , protocol_(hdr->protocol())
    {
        static_assert(sizeof(*this) == 40, "");
    }

    void checksum(std::size_t & sum) const noexcept
    {
        auto const * buf = reinterpret_cast<std::uint16_t const *>(this);
        sum += buf[ 0]; sum += buf[ 1]; sum += buf[ 2]; sum += buf[ 3]; sum += buf[ 4];
        sum += buf[ 5]; sum += buf[ 6]; sum += buf[ 7]; sum += buf[ 8]; sum += buf[ 9];
        sum += buf[10]; sum += buf[11]; sum += buf[12]; sum += buf[13]; sum += buf[14];
        sum += buf[15]; sum += buf[16]; sum += buf[17]; sum += buf[18]; sum += buf[19];
    }

private:
    internet6_address source_;
    internet6_address destinate_;
    std::uint32_t length_;
    std::uint16_t dummy1_;
    std::uint8_t dummy2_;
    std::uint8_t protocol_;
} __attribute__((__packed__));


template <>
class is_match_condition<ethernet_header, ipv6_header>
{
public:
    bool operator()(ethernet_header const & from, ipv6_header const & rhs) const
    {
        return from.protocol() == ethernet_header::ipv6;
    }
};

}  // detail

} }
