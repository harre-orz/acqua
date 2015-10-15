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
    static char * to_string(T const & bytes, char (&buf)[N])
    {
        return buf + std::sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
    }

    template <typename It, typename T>
    static void from_string(It beg, It end, T & bytes, boost::system::error_code & ec)
    {
        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 10, 1, 3> dec;
        if (qi::parse(beg, end, dec >> '.' >> dec >> '.' >> dec >> '.' >> dec,
                      bytes[0], bytes[1], bytes[2], bytes[3]) && beg == end)
            return;
        ec.assign(EAFNOSUPPORT, boost::system::generic_category());
        bytes[0] = bytes[1] = bytes[2] = bytes[3] = 0;
    }
};

}  // detail

inline internet4_address::internet4_address()
{
    static_assert(sizeof(*this) == 4, "");
    static_assert(sizeof(bytes_type) == 4, "");

    using namespace std;
    memset(bytes_.data(), 0, 4);
}

inline internet4_address::internet4_address(internet4_address const &) = default;

inline internet4_address::internet4_address(internet4_address &&) = default;

inline internet4_address::internet4_address(bytes_type const & bytes)
{
    using namespace std;
    memcpy(bytes_.data(), bytes.data(), 4);
}

inline internet4_address::internet4_address(char const addr[4])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 4);
}

inline internet4_address::internet4_address(signed char const addr[4])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 4);
}

inline internet4_address::internet4_address(unsigned char const addr[4])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 4);
}

inline internet4_address::internet4_address(struct ::in_addr const & addr)
{
    using namespace std;
    memcpy(bytes_.data(), &addr, 4);
}

inline internet4_address::internet4_address(boost::asio::ip::address_v4 const & addr)
    : internet4_address(addr.to_ulong())
{
}

inline internet4_address::internet4_address(std::uint32_t addr)
{
    bytes_[0] = reinterpret_cast<std::uint8_t const *>(&addr)[3];
    bytes_[1] = reinterpret_cast<std::uint8_t const *>(&addr)[2];
    bytes_[2] = reinterpret_cast<std::uint8_t const *>(&addr)[1];
    bytes_[3] = reinterpret_cast<std::uint8_t const *>(&addr)[0];
}

inline auto internet4_address::operator=(internet4_address const &) -> internet4_address & = default;

inline auto internet4_address::operator=(internet4_address &&) -> internet4_address & = default;

inline auto internet4_address::operator++() -> internet4_address &
{
    detail::address_impl<internet4_address>::incr(bytes_);
    return *this;
}

inline auto internet4_address::operator--() -> internet4_address &
{
    detail::address_impl<internet4_address>::decr(bytes_);
    return *this;
}

inline auto internet4_address::operator+=(long int num) -> internet4_address &
{
    detail::address_impl<internet4_address>::add(bytes_, num);
    return *this;
}

inline auto internet4_address::operator-=(long int num) -> internet4_address &
{
    detail::address_impl<internet4_address>::sub(bytes_, num);
    return *this;
}

inline internet4_address::operator ::in_addr() const
{
    return reinterpret_cast<::in_addr const &>(bytes_);
}

inline internet4_address::operator boost::asio::ip::address_v4() const
{
    return boost::asio::ip::address_v4(to_ulong());
}

inline auto internet4_address::is_unspecified() const -> bool
{
    return bytes_[0] == 0 && bytes_[1] == 0 && bytes_[2] == 0 && bytes_[3] == 0;
}

inline auto internet4_address::is_loopback() const -> bool
{
    return (bytes_[0] & 0xFF) == 0x7F;
}

inline auto internet4_address::is_class_a() const -> bool
{
    return (bytes_[0] & 0x80) == 0;
}

inline auto internet4_address::is_class_b() const -> bool
{
    return (bytes_[0] & 0xC0) == 0x80;
}

inline auto internet4_address::is_class_c() const -> bool
{
    return (bytes_[0] & 0xE0) == 0xC0;
}

inline auto internet4_address::is_multicast() const -> bool
{
    return (bytes_[0] & 0xF0) == 0xE0;
}

inline auto internet4_address::is_link_local() const -> bool
{
    return (bytes_[0] & 0xFF) == 0xA9 && (bytes_[1] & 0xFF) == 0xFE;
}

inline auto internet4_address::is_netmask() const -> bool
{
    return detail::address_impl<internet4_address>::in_mask(bytes_);
}

inline auto internet4_address::to_bytes() const -> bytes_type
{
    return bytes_;
}

inline auto internet4_address::to_string() const -> std::string
{
    char buf[4*4];
    detail::address_impl<internet4_address>::to_string(bytes_, buf);
    return buf;
}

inline auto internet4_address::to_ulong() const -> std::uint32_t
{
    std::uint32_t sum = 0;
    for(auto it = bytes_.begin(); it != bytes_.end(); ++it) {
        sum <<= 8;
        sum += (std::uint8_t)*it;
    }
    return sum;
}

inline auto internet4_address::from_string(std::string const & str) -> internet4_address
{
    internet4_address addr;
    boost::system::error_code ec;
    detail::address_impl<internet4_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

inline auto internet4_address::from_string(std::string const & str, boost::system::error_code & ec) -> internet4_address
{
    internet4_address addr;
    detail::address_impl<internet4_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    return addr;
}

inline auto internet4_address::from_string(char const * str) -> internet4_address
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

inline auto internet4_address::from_string(char const * str, boost::system::error_code & ec) -> internet4_address
{
    internet4_address addr;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
    } else {
        detail::address_impl<internet4_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
    }
    return addr;
}

inline auto operator==(internet4_address const & lhs, internet4_address const & rhs) -> bool
{
    return lhs.bytes_ == rhs.bytes_;
}

inline auto operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) -> bool
{
    return lhs.to_ulong() == rhs.to_ulong();
}

inline auto operator<(internet4_address const & lhs, internet4_address const & rhs) -> bool
{
    return lhs.bytes_ < rhs.bytes_;
}

inline auto operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) -> bool
{
    return lhs.to_ulong() < rhs.to_ulong();
}

template <typename Ch, typename Tr>
inline auto operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs) -> std::basic_ostream<Ch, Tr> &
{
    char buf[4*4];
    char * end = detail::address_impl<internet4_address>::to_string(rhs.bytes_, buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

inline auto hash_value(internet4_address const & rhs) -> std::size_t
{
    return std::accumulate(rhs.bytes_.begin(), rhs.bytes_.end(), (uint)0);
}

inline auto netmask_length(internet4_address const & rhs) -> int
{
    return detail::address_impl<internet4_address>::netmask_length(rhs.bytes_);
}

} }
