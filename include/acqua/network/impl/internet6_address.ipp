/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace network {

namespace detail {

template <typename It>
inline constexpr void from_uint64s(std::uint64_t low, std::uint64_t high, It it)
{
    for(int i = 0; i < 8; ++i) {
        *it++ = (low & 0xFF);
        low >>= 8;
    }
    for(int i = 0; i < 8; ++i) {
        *it++ = (high & 0xFF);
        high >>= 8;
    }
}

inline void from_string(char const * str, internet6_address::bytes_type & addr, boost::system::error_code & ec)
{
    if (::inet_pton(AF_INET6, str, addr.data()) != 1)
        ec.assign(EAFNOSUPPORT, boost::system::generic_category());
}

template <uint N>
inline char * to_string(internet6_address::bytes_type const & bytes, char (&buf)[N])
{
    ::inet_ntop(AF_INET6, bytes.data(), buf, N);
    return buf + std::strlen(buf);
}

inline std::size_t hash_func(std::uint32_t, void const * data)
{
    return ((std::uint32_t const *)data)[0]
        ^  ((std::uint32_t const *)data)[1]
        ^  ((std::uint32_t const *)data)[2]
        ^  ((std::uint32_t const *)data)[3];
}

inline std::size_t hash_func(std::uint64_t, void const * data)
{
    return ((std::uint64_t const *)data)[0]
        ^  ((std::uint64_t const *)data)[1];
}

}  // detail

internet6_address::internet6_address()
{
    static_assert(sizeof(*this) == 16, "");
    static_assert(sizeof(bytes_type) == 16, "");
    addr_.fill(0);
}

internet6_address::internet6_address(internet6_address const & rhs)
    : addr_(rhs.addr_)
{
}

internet6_address::internet6_address(internet6_address && rhs)
    : addr_(rhs.addr_)
{
}

internet6_address::internet6_address(bytes_type const & addr)
    : addr_(addr)
{
}

internet6_address::internet6_address(char const addr[16])
{
    std::memcpy(addr_.data(), addr, 16);
}

internet6_address::internet6_address(signed char const addr[16])
{
    std::memcpy(addr_.data(), addr, 16);
}

internet6_address::internet6_address(unsigned char const addr[16])
{
    std::memcpy(addr_.data(), addr, 16);
}

internet6_address::internet6_address(struct ::in6_addr const & addr)
{
    std::memcpy(addr_.data(), &addr, 16);
}

internet6_address::internet6_address(boost::asio::ip::address_v6 const & rhs)
    : addr_(rhs.to_bytes())
{
}

internet6_address::internet6_address(std::uint64_t low)
{
    detail::from_uint64s(low, 0, addr_.rbegin());
}

internet6_address::internet6_address(std::uint64_t high, std::uint64_t low)
{
    detail::from_uint64s(low, high, addr_.rbegin());
}

internet6_address & internet6_address::operator++()
{
    for(auto it = addr_.rbegin(); it != addr_.rend() && ++(*it++) == 0x00;);
    return *this;
}

internet6_address & internet6_address::operator--()
{
    for(auto it = addr_.rbegin(); it != addr_.rend() && --(*it++) == 0xFF;);
    return *this;
}

internet6_address & internet6_address::operator+=(long int num)
{
    if (num < 0)
        return operator-=(-num);
    for(auto it = addr_.rbegin(); it != addr_.rend() && num; ++it) {
        *it += (num & 0xff);
        num >>= 8;
    }
    return *this;
}

internet6_address & internet6_address::operator-=(long int num)
{
    if (num < 0)
        return operator+=(-num);
    for(auto it = addr_.rbegin(); it != addr_.rend() && num; ++it) {
        *it -= (num & 0xFF);
        num >>= 8;
    }
    return *this;
}

internet6_address::operator ::in6_addr() const
{
    return *reinterpret_cast<::in6_addr const *>(addr_.data());
}

internet6_address::operator boost::asio::ip::address_v6() const
{
    return boost::asio::ip::address_v6(addr_);
}

bool internet6_address::is_unspecified() const
{
    return addr_[ 0] == 0 && addr_[ 1] == 0 && addr_[ 2] == 0 && addr_[ 3] == 0
        && addr_[ 4] == 0 && addr_[ 5] == 0 && addr_[ 6] == 0 && addr_[ 7] == 0
        && addr_[ 8] == 0 && addr_[ 9] == 0 && addr_[10] == 0 && addr_[11] == 0
        && addr_[12] == 0 && addr_[13] == 0 && addr_[14] == 0 && addr_[15] == 0;
}

bool internet6_address::is_loopback() const
{
    return addr_[ 0] == 0 && addr_[ 1] == 0 && addr_[ 2] == 0 && addr_[ 3] == 0
        && addr_[ 4] == 0 && addr_[ 5] == 0 && addr_[ 6] == 0 && addr_[ 7] == 0
        && addr_[ 8] == 0 && addr_[ 9] == 0 && addr_[10] == 0 && addr_[11] == 0
        && addr_[12] == 0 && addr_[13] == 0 && addr_[14] == 0 && addr_[15] == 1;
}

bool internet6_address::is_link_local() const
{
    return addr_[0] == 0xFE && (addr_[1] & 0xC0) == 0x80;
}

bool internet6_address::is_site_local() const
{
    return addr_[0] == 0xFE && (addr_[1] & 0xC0) == 0xC0;
}

bool internet6_address::is_v4_mapped() const
{
    return addr_[ 0] == 0 && addr_[ 1] == 0 && addr_[ 2] == 0 && addr_[ 3] == 0
        && addr_[ 4] == 0 && addr_[ 5] == 0 && addr_[ 6] == 0 && addr_[ 7] == 0
        && addr_[ 8] == 0 && addr_[ 9] == 0 && addr_[10] == 0xFF && addr_[11] == 0xFF;
}

bool internet6_address::is_v4_compatible() const
{
    return addr_[ 0] == 0 && addr_[ 1] == 0 && addr_[ 2] == 0 && addr_[ 3] == 0
        && addr_[ 4] == 0 && addr_[ 5] == 0 && addr_[ 6] == 0 && addr_[ 7] == 0
        && addr_[ 8] == 0 && addr_[ 9] == 0 && addr_[10] == 0 && addr_[11] == 0
        && !(addr_[12] == 0 && addr_[13] == 0 && addr_[14] == 0 && (addr_[15] == 0 || addr_[15] == 1));
}

bool internet6_address::is_multicast() const
{
    return addr_[0] == 0xFF;
}

bool internet6_address::is_multicast_global() const
{
    return addr_[0] == 0xFF && (addr_[1] & 0x0F) == 0x0E;
}

bool internet6_address::is_multicast_link_local() const
{
    return addr_[0] == 0xFF && (addr_[1] & 0x0F) == 0x02;
}

bool internet6_address::is_multicast_node_local() const
{
    return addr_[0] == 0xFF && (addr_[1] & 0x0F) == 0x01;
}

bool internet6_address::is_multicast_org_local() const
{
    return addr_[0] == 0xFF && (addr_[1] & 0x0F) == 0x08;
}

bool internet6_address::is_multicast_site_local() const
{
    return addr_[0] == 0xFF && (addr_[1] & 0x0F) == 0x05;
}

bool internet6_address::is_netmask() const
{
    auto it = addr_.begin();
    while(it != addr_.end()) {
        switch(*it++) {
            case 0b11111111:
                ;
            case 0b10000000:
            case 0b11000000:
            case 0b11100000:
            case 0b11110000:
            case 0b11111000:
            case 0b11111100:
            case 0b11111110:
                return std::accumulate(it, addr_.end(), 0) == 0;
            default:
                return false;
        }
    }
    return true;
}

internet6_address::bytes_type internet6_address::to_bytes() const
{
    return addr_;
}

std::string internet6_address::to_string() const
{
    return boost::lexical_cast<std::string>(*this);
}

internet6_address internet6_address::from_string(std::string const & str)
{
    internet6_address addr;
    boost::system::error_code ec;
    detail::from_string(str.c_str(), addr.addr_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

internet6_address internet6_address::from_string(std::string const & str, boost::system::error_code & ec)
{
    internet6_address addr;
    detail::from_string(str.c_str(), addr.addr_, ec);
    return addr;
}

internet6_address internet6_address::from_string(char const * str)
{
    internet6_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
        boost::asio::detail::throw_error(ec, "from_string");
    }
    detail::from_string(str, addr.addr_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

internet6_address internet6_address::from_string(char const * str, boost::system::error_code & ec)
{
    internet6_address addr;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
        return addr;
    }
    detail::from_string(str, addr.addr_, ec);
    return addr;
}

bool operator==(internet6_address const & lhs, internet6_address const & rhs)
{
    return lhs.addr_ == rhs.addr_;
}

bool operator==(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs)
{
    return lhs.addr_ == rhs.to_bytes();
}

bool operator<(internet6_address const & lhs, internet6_address const & rhs)
{
    return lhs.addr_ < rhs.addr_;
}

bool operator<(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs)
{
    return lhs.addr_ < rhs.to_bytes();
}

std::size_t hash_value(internet6_address const & rhs)
{
    return detail::hash_func(std::size_t(), rhs.addr_.data());
}

template <typename Ch, typename Tr>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs)
{
    char buf[128];
    char * end = detail::to_string(rhs.addr_, buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

} }
