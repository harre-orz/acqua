/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/address_impl.hpp>

namespace acqua { namespace network {

namespace detail {

template <>
struct address_impl<internet4_address>
    : address_impl_base
{
    template <typename T, uint N>
    static char * to_string(T const & bytes, char (&buf)[N]) noexcept
    {
        return buf + std::sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    }

    template <typename It, typename T>
    static void from_string(It beg, It end, T & bytes, boost::system::error_code & ec) noexcept
    {
        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 10, 1, 3> dec;
        if (qi::parse(beg, end, dec >> '.' >> dec >> '.' >> dec >> '.' >> dec,
                      bytes[0], bytes[1], bytes[2], bytes[3]) && beg == end)
            return;
        ec = make_error_code(boost::system::errc::address_family_not_supported);
        bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0;
    }
};

}  // detail

inline constexpr internet4_address::internet4_address() noexcept
    : bytes_({{0,0,0,0}})
{
    static_assert(sizeof(*this) == 4, "");
    static_assert(sizeof(bytes_type) == 4, "");
}

inline constexpr internet4_address::internet4_address(bytes_type const & bytes) noexcept
    : bytes_(bytes)
{
}

inline constexpr internet4_address::internet4_address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept
    : bytes_({{a,b,c,d}})
{
}

inline internet4_address::internet4_address(struct ::in_addr const & addr) noexcept
    : bytes_(*reinterpret_cast<bytes_type const *>(&addr))
{
}

inline internet4_address::internet4_address(boost::asio::ip::address_v4 const & addr) noexcept
    : internet4_address(static_cast<std::uint32_t>(addr.to_ulong()))
{
}

inline internet4_address::internet4_address(std::uint32_t addr) noexcept
    : bytes_({{
#ifdef BOOST_BIG_ENDIAN
        reinterpret_cast<std::uint8_t const *>(&addr)[0],
        reinterpret_cast<std::uint8_t const *>(&addr)[1],
        reinterpret_cast<std::uint8_t const *>(&addr)[2],
        reinterpret_cast<std::uint8_t const *>(&addr)[3]
#else
        reinterpret_cast<std::uint8_t const *>(&addr)[3],
        reinterpret_cast<std::uint8_t const *>(&addr)[2],
        reinterpret_cast<std::uint8_t const *>(&addr)[1],
        reinterpret_cast<std::uint8_t const *>(&addr)[0]
#endif
            }})
{
}

inline internet4_address & internet4_address::operator++() noexcept
{
    detail::address_impl<internet4_address>::incr(bytes_);
    return *this;
}

inline internet4_address & internet4_address::operator--() noexcept
{
    detail::address_impl<internet4_address>::decr(bytes_);
    return *this;
}

inline internet4_address & internet4_address::operator+=(long int num) noexcept
{
    detail::address_impl<internet4_address>::add(bytes_, num);
    return *this;
}

inline internet4_address & internet4_address::operator-=(long int num) noexcept
{
    detail::address_impl<internet4_address>::sub(bytes_, num);
    return *this;
}

inline internet4_address::operator ::in_addr() const noexcept
{
    return reinterpret_cast<::in_addr const &>(bytes_);
}

inline internet4_address::operator boost::asio::ip::address_v4() const noexcept
{
    return boost::asio::ip::address_v4(to_ulong());
}

inline bool internet4_address::is_unspecified() const noexcept
{
    return bytes_[0] == 0 && bytes_[1] == 0 && bytes_[2] == 0 && bytes_[3] == 0;
}

inline bool internet4_address::is_loopback() const noexcept
{
    return (bytes_[0] & 0xFF) == 0x7F;
}

inline bool internet4_address::is_class_a() const noexcept
{
    return (bytes_[0] & 0x80) == 0;
}

inline bool internet4_address::is_class_b() const noexcept
{
    return (bytes_[0] & 0xC0) == 0x80;
}

inline bool internet4_address::is_class_c() const noexcept
{
    return (bytes_[0] & 0xE0) == 0xC0;
}

inline bool internet4_address::is_multicast() const noexcept
{
    return (bytes_[0] & 0xF0) == 0xE0;
}

inline bool internet4_address::is_link_local() const noexcept
{
    return (bytes_[0] & 0xFF) == 0xA9 && (bytes_[1] & 0xFF) == 0xFE;
}

inline bool internet4_address::is_netmask() const noexcept
{
    return detail::address_impl<internet4_address>::in_mask(bytes_);
}

inline constexpr auto internet4_address::to_bytes() const noexcept -> bytes_type
{
    return bytes_;
}

inline std::string internet4_address::to_string() const
{
    char buf[4*4];
    detail::address_impl<internet4_address>::to_string(bytes_, buf);
    return buf;
}

inline std::uint32_t internet4_address::to_ulong() const noexcept
{
    std::uint32_t sum = 0;
    for(auto it = bytes_.begin(); it != bytes_.end(); ++it) {
        sum <<= 8;
        sum += static_cast<std::uint8_t>(*it);
    }
    return sum;
}

inline void internet4_address::checksum(std::size_t & sum) const noexcept
{
    auto * buf = reinterpret_cast<std::uint16_t const *>(bytes_.data());
    for(uint i = 0; i < sizeof(*this)/sizeof(*buf); ++i)
        sum += *buf++;
}

inline constexpr internet4_address internet4_address::any() noexcept
{
    return internet4_address();
}

inline constexpr internet4_address internet4_address::broadcast() noexcept
{
    return bytes_type({{255,255,255,255}});
}

inline constexpr internet4_address internet4_address::loopback() noexcept
{
    return bytes_type({{127,0,0,1}});
}

inline internet4_address internet4_address::from_string(std::string const & str)
{
    internet4_address addr;
    boost::system::error_code ec;
    detail::address_impl<internet4_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

inline internet4_address internet4_address::from_string(std::string const & str, boost::system::error_code & ec) noexcept
{
    internet4_address addr;
    detail::address_impl<internet4_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    return addr;
}

inline internet4_address internet4_address::from_string(char const * str)
{
    internet4_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
        boost::asio::detail::throw_error(ec, "from_string");
    } else {
        detail::address_impl<internet4_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
        boost::asio::detail::throw_error(ec, "from_string");
    }
    return addr;
}

inline internet4_address internet4_address::from_string(char const * str, boost::system::error_code & ec) noexcept
{
    internet4_address addr;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
    } else {
        detail::address_impl<internet4_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
    }
    return addr;
}

inline internet4_address internet4_address::from_voidptr(void const * ptr) noexcept
{
    return *reinterpret_cast<bytes_type const *>(ptr);
}

inline bool operator==(internet4_address const & lhs, internet4_address const & rhs) noexcept
{
    return lhs.bytes_ == rhs.bytes_;
}

inline bool operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) noexcept
{
    return lhs.to_ulong() == rhs.to_ulong();
}

inline bool operator<(internet4_address const & lhs, internet4_address const & rhs) noexcept
{
    return lhs.bytes_ < rhs.bytes_;
}

inline bool operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) noexcept
{
    return lhs.to_ulong() < rhs.to_ulong();
}

template <typename Ch, typename Tr>
inline std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs)
{
    char buf[4*4];
    char * end = detail::address_impl<internet4_address>::to_string(rhs.bytes_, buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

inline std::size_t hash_value(internet4_address const & rhs) noexcept
{
    return std::accumulate(rhs.bytes_.begin(), rhs.bytes_.end(), 0ul);
}

inline int netmask_length(internet4_address const & rhs) noexcept
{
    return detail::address_impl<internet4_address>::netmask_length(rhs.bytes_);
}

} }
