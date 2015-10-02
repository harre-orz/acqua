/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/address_impl.hpp>

namespace acqua { namespace network {

namespace detail {

template <>
struct address_impl<internet6_address>
    : address_impl_base
{
    template <typename T, uint N>
    static char * to_string(T const & bytes, char (&buf)[N])
    {
        ::inet_ntop(AF_INET6, bytes.data(), buf, N);
        return buf + std::strlen(buf);
    }

    template <typename T>
    static void from_string(char const * str, T & bytes, boost::system::error_code & ec)
    {
        if (::inet_pton(AF_INET6, str, bytes.data()) != 1)
            ec.assign(EAFNOSUPPORT, boost::system::generic_category());
    }

    template <typename It>
    static void from_uint64s(std::uint64_t low, std::uint64_t high, It it)
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

    template <typename T>
    static std::size_t hash_func(void const * data);
};

template <>
std::size_t address_impl<internet6_address>::hash_func<std::uint32_t>(void const * data)
{
    return ((std::uint32_t const *)data)[0]
        ^  ((std::uint32_t const *)data)[1]
        ^  ((std::uint32_t const *)data)[2]
        ^  ((std::uint32_t const *)data)[3];
}

template <>
std::size_t address_impl<internet6_address>::hash_func<std::uint64_t>(void const * data)
{
    return ((std::uint64_t const *)data)[0]
        ^  ((std::uint64_t const *)data)[1];
}

}  // detail

internet6_address::internet6_address()
{
    static_assert(sizeof(*this) == 16, "");
    static_assert(sizeof(bytes_type) == 16, "");
    bytes_.fill(0);
}

internet6_address::internet6_address(bytes_type const & bytes)
    : bytes_(bytes)
{
}

internet6_address::internet6_address(char const addr[16])
{
    std::memcpy(bytes_.data(), addr, 16);
}

internet6_address::internet6_address(signed char const addr[16])
{
    std::memcpy(bytes_.data(), addr, 16);
}

internet6_address::internet6_address(unsigned char const addr[16])
{
    std::memcpy(bytes_.data(), addr, 16);
}

internet6_address::internet6_address(struct ::in6_addr const & addr)
{
    std::memcpy(bytes_.data(), &addr, 16);
}

internet6_address::internet6_address(boost::asio::ip::address_v6 const & rhs)
    : bytes_(rhs.to_bytes())
{
}

internet6_address::internet6_address(std::uint64_t low)
{
    detail::address_impl<internet6_address>::from_uint64s(low, 0, bytes_.rbegin());
}

internet6_address::internet6_address(std::uint64_t high, std::uint64_t low)
{
    detail::address_impl<internet6_address>::from_uint64s(low, high, bytes_.rbegin());
}

internet6_address & internet6_address::operator++()
{
    detail::address_impl<internet6_address>::incr(bytes_);
    return *this;
}

internet6_address & internet6_address::operator--()
{
    detail::address_impl<internet6_address>::decr(bytes_);
    return *this;
}

internet6_address & internet6_address::operator+=(long int num)
{
    detail::address_impl<internet6_address>::add(bytes_, num);
    return *this;
}

internet6_address & internet6_address::operator-=(long int num)
{
    detail::address_impl<internet6_address>::sub(bytes_, num);
    return *this;
}

internet6_address::operator ::in6_addr() const
{
    return *reinterpret_cast<::in6_addr const *>(bytes_.data());
}

internet6_address::operator boost::asio::ip::address_v6() const
{
    return boost::asio::ip::address_v6(bytes_);
}

bool internet6_address::is_unspecified() const
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && bytes_[15] == 0;
}

bool internet6_address::is_loopback() const
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && bytes_[15] == 1;
}

bool internet6_address::is_link_local() const
{
    return bytes_[0] == 0xFE && (bytes_[1] & 0xC0) == 0x80;
}

bool internet6_address::is_site_local() const
{
    return bytes_[0] == 0xFE && (bytes_[1] & 0xC0) == 0xC0;
}

bool internet6_address::is_v4_mapped() const
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0xFF && bytes_[11] == 0xFF;
}

bool internet6_address::is_v4_compatible() const
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && !(bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && (bytes_[15] == 0 || bytes_[15] == 1));
}

bool internet6_address::is_multicast() const
{
    return bytes_[0] == 0xFF;
}

bool internet6_address::is_multicast_global() const
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x0E;
}

bool internet6_address::is_multicast_link_local() const
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x02;
}

bool internet6_address::is_multicast_node_local() const
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x01;
}

bool internet6_address::is_multicast_org_local() const
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x08;
}

bool internet6_address::is_multicast_site_local() const
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x05;
}

bool internet6_address::is_netmask() const
{
    return detail::address_impl<internet6_address>::in_mask(bytes_);
}

internet6_address::bytes_type internet6_address::to_bytes() const
{
    return bytes_;
}

std::string internet6_address::to_string() const
{
    char buf[64];
    detail::address_impl<internet6_address>::to_string(bytes_, buf);
    return buf;
}

internet6_address internet6_address::from_string(std::string const & str)
{
    internet6_address addr;
    boost::system::error_code ec;
    detail::address_impl<internet6_address>::from_string(str.c_str(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

internet6_address internet6_address::from_string(std::string const & str, boost::system::error_code & ec)
{
    internet6_address addr;
    detail::address_impl<internet6_address>::from_string(str.c_str(), addr.bytes_, ec);
    return addr;
}

internet6_address internet6_address::from_string(char const * str)
{
    internet6_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
        boost::asio::detail::throw_error(ec, "from_string");
    } else {
        detail::address_impl<internet6_address>::from_string(str, addr.bytes_, ec);
        boost::asio::detail::throw_error(ec, "from_string");
    }
    return addr;
}

internet6_address internet6_address::from_string(char const * str, boost::system::error_code & ec)
{
    internet6_address addr;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
    } else {
        detail::address_impl<internet6_address>::from_string(str, addr.bytes_, ec);
    }
    return addr;
}

internet6_address internet6_address::from_voidptr(void const * ptr)
{
    internet6_address addr;
    if (ptr != nullptr) {
        using namespace std;
        memcpy(addr.bytes_.data(), ptr, 16);
    }
    return addr;
}

bool operator==(internet6_address const & lhs, internet6_address const & rhs)
{
    return lhs.bytes_ == rhs.bytes_;
}

bool operator==(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs)
{
    return lhs.bytes_ == rhs.to_bytes();
}

bool operator<(internet6_address const & lhs, internet6_address const & rhs)
{
    return lhs.bytes_ < rhs.bytes_;
}

bool operator<(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs)
{
    return lhs.bytes_ < rhs.to_bytes();
}

template <typename Ch, typename Tr>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs)
{
    char buf[64];
    char * end = detail::address_impl<internet6_address>::to_string(rhs.bytes_, buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

std::size_t hash_value(internet6_address const & rhs)
{
    return detail::address_impl<internet6_address>::template hash_func<std::size_t>(rhs.bytes_.data());
}

internet6_address::masklen_type netmask_length(internet6_address const & rhs)
{
    return detail::address_impl<internet6_address>::netmask_length(rhs.bytes_);
}

} }
