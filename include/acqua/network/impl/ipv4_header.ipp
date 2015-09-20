#pragma once

#include <acqua/network/ethernet_header.hpp>
#include <acqua/network/ipv4_header.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network {

std::size_t ipv4_header::size() const
{
    return value_type::ip_hl * 4;
}


template <typename It>
void ipv4_header::shrink(It & end) const
{
    int len = (reinterpret_cast<std::uint8_t const *>(&(*end)) - reinterpret_cast<std::uint8_t const *>(this)) - ntohs(value_type
                                                                                                                       ::ip_len);
    if (0 < len && len < (reinterpret_cast<std::uint8_t const *>(&(*end)) - reinterpret_cast<std::uint8_t const *>(this)))
        std::advance(end, -len);
}


std::ostream & operator<<(std::ostream & os, ipv4_header const & rhs)
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
       << " size=" << rhs.size()
       << " check=" << std::hex << rhs.checksum() << std::dec
       << " ver=" << rhs.version()
       << " hlen=" << rhs.header_length()
       << " tos=" << rhs.type_of_service()
       << " tlen=" << rhs.total_length()
       << " id=" << rhs.id()
       << " tol=" << rhs.time_of_live();
    return os;
}

namespace detail {

template <>
class pseudo_header<ipv4_header>
{
public:
    explicit pseudo_header(ipv4_header const * hdr, std::size_t size)
        : source_(hdr->source())
        , destinate_(hdr->destinate())
        , dummy_(0)
        , protocol_(hdr->protocol())
        , length_(htons(size))
    {
        static_assert(sizeof(*this) == 12, "");
    }

    void checksum(std::size_t & sum) const
    {
        auto const * buf = reinterpret_cast<std::uint16_t const *>(this);
        sum += buf[0];
        sum += buf[1];
        sum += buf[2];
        sum += buf[3];
        sum += buf[4];
        sum += buf[5];
    }

private:
    internet4_address source_;
    internet4_address destinate_;
    std::uint8_t dummy_;
    std::uint8_t protocol_;
    std::uint16_t length_;
} __attribute__((__packed__));


template <>
class is_match_condition<ethernet_header, ipv4_header>
{
public:
    bool operator()(ethernet_header const & from, ipv4_header const &) const
    {
        return from.protocol() == ethernet_header::ip;
    }
};

}  // detail

} }
