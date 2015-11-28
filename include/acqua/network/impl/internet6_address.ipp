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
    static char * to_string(T const & bytes, char (&buf)[N]) noexcept
    {
        ::inet_ntop(AF_INET6, bytes.data(), buf, N);
        return buf + std::strlen(buf);
    }

    template <typename T>
    static void from_string(char const * str, T & bytes, boost::system::error_code & ec) noexcept
    {
        if (::inet_pton(AF_INET6, str, bytes.data()) != 1)
            ec = make_error_code(boost::system::errc::address_family_not_supported);
    }

    template <typename It>
    static void from_uint64s(std::uint64_t low, std::uint64_t high, It it) noexcept
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
    static std::size_t hash_func(void const * data) noexcept;
};

template <>
inline std::size_t address_impl<internet6_address>::hash_func<std::uint32_t>(void const * data) noexcept
{
    return static_cast<std::uint32_t const *>(data)[0]
        ^  static_cast<std::uint32_t const *>(data)[1]
        ^  static_cast<std::uint32_t const *>(data)[2]
        ^  static_cast<std::uint32_t const *>(data)[3];
}

template <>
inline std::size_t address_impl<internet6_address>::hash_func<std::uint64_t>(void const * data) noexcept
{
    return static_cast<std::uint64_t const *>(data)[0]
        ^  static_cast<std::uint64_t const *>(data)[1];
}

}  // detail

inline constexpr internet6_address::internet6_address() noexcept
    : bytes_{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}
{
    static_assert(sizeof(*this) == 16, "");
    static_assert(sizeof(bytes_type) == 16, "");
}

inline constexpr internet6_address::internet6_address(bytes_type const & bytes) noexcept
    : bytes_(bytes)
{
}

inline internet6_address::internet6_address(struct ::in6_addr const & addr) noexcept
    : bytes_(*reinterpret_cast<bytes_type const *>(&addr))
{
}

inline internet6_address::internet6_address(boost::asio::ip::address_v6 const & rhs) noexcept
    : bytes_(rhs.to_bytes())
{
}

inline internet6_address & internet6_address::operator++() noexcept
{
    detail::address_impl<internet6_address>::incr(bytes_);
    return *this;
}

inline internet6_address & internet6_address::operator--() noexcept
{
    detail::address_impl<internet6_address>::decr(bytes_);
    return *this;
}

inline internet6_address & internet6_address::operator+=(long int num) noexcept
{
    detail::address_impl<internet6_address>::add(bytes_, num);
    return *this;
}

inline internet6_address & internet6_address::operator-=(long int num) noexcept
{
    detail::address_impl<internet6_address>::sub(bytes_, num);
    return *this;
}

inline internet6_address::operator ::in6_addr() const noexcept
{
    return *reinterpret_cast<::in6_addr const *>(bytes_.data());
}

inline internet6_address::operator boost::asio::ip::address_v6() const noexcept
{
    return boost::asio::ip::address_v6(bytes_);
}

inline bool internet6_address::is_unspecified() const noexcept
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && bytes_[15] == 0;
}

inline bool internet6_address::is_loopback() const noexcept
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && bytes_[15] == 1;
}

inline bool internet6_address::is_link_local() const noexcept
{
    return bytes_[0] == 0xFE && (bytes_[1] & 0xC0) == 0x80;
}

inline bool internet6_address::is_site_local() const noexcept
{
    return bytes_[0] == 0xFE && (bytes_[1] & 0xC0) == 0xC0;
}

inline bool internet6_address::is_v4_mapped() const noexcept
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0xFF && bytes_[11] == 0xFF;
}

inline bool internet6_address::is_v4_compatible() const noexcept
{
    return bytes_[ 0] == 0 && bytes_[ 1] == 0 && bytes_[ 2] == 0 && bytes_[ 3] == 0
        && bytes_[ 4] == 0 && bytes_[ 5] == 0 && bytes_[ 6] == 0 && bytes_[ 7] == 0
        && bytes_[ 8] == 0 && bytes_[ 9] == 0 && bytes_[10] == 0 && bytes_[11] == 0
        && !(bytes_[12] == 0 && bytes_[13] == 0 && bytes_[14] == 0 && (bytes_[15] == 0 || bytes_[15] == 1));
}

inline bool internet6_address::is_multicast() const noexcept
{
    return bytes_[0] == 0xFF;
}

inline bool internet6_address::is_multicast_global() const noexcept
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x0E;
}

inline bool internet6_address::is_multicast_link_local() const noexcept
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x02;
}

inline bool internet6_address::is_multicast_node_local() const noexcept
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x01;
}

inline bool internet6_address::is_multicast_org_local() const noexcept
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x08;
}

inline bool internet6_address::is_multicast_site_local() const noexcept
{
    return bytes_[0] == 0xFF && (bytes_[1] & 0x0F) == 0x05;
}

inline bool internet6_address::is_netmask() const noexcept
{
    return detail::address_impl<internet6_address>::in_mask(bytes_);
}

inline internet6_address::bytes_type internet6_address::to_bytes() const noexcept
{
    return bytes_;
}

inline std::string internet6_address::to_string() const
{
    char buf[64];
    detail::address_impl<internet6_address>::to_string(bytes_, buf);
    return buf;
}

inline void internet6_address::checksum(std::size_t & sum) const noexcept
{
    auto * buf = reinterpret_cast<std::uint16_t const *>(bytes_.data());
    for(uint i = 0; i < sizeof(*this)/sizeof(*buf); ++i)
        sum += *buf++;
}

inline constexpr internet6_address internet6_address::any() noexcept
{
    return internet6_address();
}

inline constexpr internet6_address internet6_address::loopback() noexcept
{
    return bytes_type{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}};
}

inline internet6_address internet6_address::from_string(std::string const & str)
{
    internet6_address addr;
    boost::system::error_code ec;
    detail::address_impl<internet6_address>::from_string(str.c_str(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

inline internet6_address internet6_address::from_string(std::string const & str, boost::system::error_code & ec) noexcept
{
    internet6_address addr;
    detail::address_impl<internet6_address>::from_string(str.c_str(), addr.bytes_, ec);
    return addr;
}

inline internet6_address internet6_address::from_string(char const * str)
{
    internet6_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec = make_error_code(boost::system::errc::invalid_argument);
        boost::asio::detail::throw_error(ec, "from_string");
    } else {
        detail::address_impl<internet6_address>::from_string(str, addr.bytes_, ec);
        boost::asio::detail::throw_error(ec, "from_string");
    }
    return addr;
}

inline internet6_address internet6_address::from_string(char const * str, boost::system::error_code & ec) noexcept
{
    internet6_address addr;
    if (str == nullptr) {
        ec = make_error_code(boost::system::errc::invalid_argument);
    } else {
        detail::address_impl<internet6_address>::from_string(str, addr.bytes_, ec);
    }
    return addr;
}

inline internet6_address internet6_address::from_voidptr(void const * ptr) noexcept
{
    return *reinterpret_cast<bytes_type const *>(ptr);
}

inline bool operator==(internet6_address const & lhs, internet6_address const & rhs) noexcept
{
    return lhs.bytes_ == rhs.bytes_;
}

inline bool operator==(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs) noexcept
{
    return lhs.bytes_ == rhs.to_bytes();
}

inline bool operator<(internet6_address const & lhs, internet6_address const & rhs) noexcept
{
    return lhs.bytes_ < rhs.bytes_;
}

inline bool operator<(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs) noexcept
{
    return lhs.bytes_ < rhs.to_bytes();
}

template <typename Ch, typename Tr>
inline std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs)
{
    char buf[64];
    char * end = detail::address_impl<internet6_address>::to_string(rhs.bytes_, buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

inline std::size_t hash_value(internet6_address const & rhs) noexcept
{
    return detail::address_impl<internet6_address>::template hash_func<std::size_t>(rhs.bytes_.data());
}

inline int  netmask_length(internet6_address const & rhs) noexcept
{
    return detail::address_impl<internet6_address>::netmask_length(rhs.bytes_);
}

} }
