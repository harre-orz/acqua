/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/detail/address_impl.hpp>
#include <boost/asio/detail/throw_error.hpp>

namespace acqua { namespace network {

namespace detail {

template <>
struct address_impl<linklayer_address>
    : address_impl_base
{
    template <typename T, uint N>
    static char * to_string(T const & bytes, char (&buf)[N]) noexcept
    {
        return buf + std::sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                                  bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
    }

    template <typename It, typename T>
    static void from_string(It beg, It end, T & bytes, boost::system::error_code & ec) noexcept
    {
        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 16, 2, 2> hex;
        qi::rule<It> sep = qi::lit(':') | qi::lit('-');
        if (qi::parse(beg, end, hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex,
                      bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]))
            return;
        ec = make_error_code(boost::system::errc::address_family_not_supported);
        bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0;
    }

    template <typename T>
    static std::size_t hash_func(void const * data) noexcept;
};

template <>
inline std::size_t address_impl<linklayer_address>::hash_func<std::uint32_t>(void const * data) noexcept
{
    return *reinterpret_cast<std::uint32_t const *>(reinterpret_cast<std::uint8_t const *>(data) + 0)
        ^  *reinterpret_cast<std::uint16_t const *>(reinterpret_cast<std::uint8_t const *>(data) + 4);
}

template <>
inline std::size_t address_impl<linklayer_address>::hash_func<std::uint64_t>(void const * data) noexcept
{
    return (*reinterpret_cast<std::uint32_t const *>(reinterpret_cast<std::uint8_t const *>(data) + 0) << 16)
        ^  (*reinterpret_cast<std::uint16_t const *>(reinterpret_cast<std::uint8_t const *>(data) + 4));
}

}  // detail

inline constexpr linklayer_address::linklayer_address() noexcept
    : bytes_({{0,0,0,0,0,0}})
{
}

inline constexpr linklayer_address::linklayer_address(bytes_type const & bytes) noexcept
    : bytes_(bytes)
{
}

inline linklayer_address & linklayer_address::operator++() noexcept
{
    detail::address_impl<linklayer_address>::incr(bytes_);
    return *this;
}

inline linklayer_address & linklayer_address::operator--() noexcept
{
    detail::address_impl<linklayer_address>::decr(bytes_);
    return *this;
}

inline linklayer_address & linklayer_address::operator+=(long int num) noexcept
{
    detail::address_impl<linklayer_address>::add(bytes_, num);
    return *this;
}

inline linklayer_address & linklayer_address::operator-=(long int num) noexcept
{
    detail::address_impl<linklayer_address>::sub(bytes_, num);
    return *this;
}

inline bool linklayer_address::is_unspecified() const noexcept
{
    return bytes_[0] == 0 && bytes_[1] == 0 && bytes_[2] == 0
        && bytes_[3] == 0 && bytes_[4] == 0 && bytes_[5] == 0;
}

inline constexpr auto linklayer_address::to_bytes() const noexcept -> bytes_type
{
    return bytes_;
}

inline std::uint32_t linklayer_address::to_oui() const noexcept
{
    std::uint32_t oui = bytes_[0];
    oui <<= 8;;
    oui |= bytes_[1];
    oui <<= 8;
    oui |= bytes_[2];
    return oui;
}

inline std::string linklayer_address::to_string() const
{
    char buf[12];
    return buf;
}

inline constexpr linklayer_address linklayer_address::any() noexcept
{
    return linklayer_address();
}

inline constexpr linklayer_address linklayer_address::broadcast() noexcept
{
    return bytes_type({{255,255,255,255,255,255}});
}

inline linklayer_address linklayer_address::from_string(std::string const & str)
{
    linklayer_address addr;
    boost::system::error_code ec;
    detail::address_impl<linklayer_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

inline linklayer_address linklayer_address::from_string(std::string const & str, boost::system::error_code & ec) noexcept
{
    linklayer_address addr;
    detail::address_impl<linklayer_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    return addr;
}

inline linklayer_address linklayer_address::from_string(char const * str)
{
    linklayer_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec = make_error_code(boost::system::errc::invalid_argument);
    } else {
        detail::address_impl<linklayer_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
        boost::asio::detail::throw_error(ec, "from_string");
    }
    return addr;
}

inline linklayer_address linklayer_address::from_string(char const * str, boost::system::error_code & ec) noexcept
{
    linklayer_address addr;
    if (str == nullptr) {
        ec = make_error_code(boost::system::errc::invalid_argument);
    } else {
        detail::address_impl<linklayer_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
    }
    return addr;
}

inline linklayer_address linklayer_address::from_voidptr(void const * ptr) noexcept
{
    return *reinterpret_cast<bytes_type const *>(ptr);
}

inline bool operator==(linklayer_address const & lhs, linklayer_address const & rhs) noexcept
{
    return lhs.bytes_ == rhs.bytes_;
}

inline bool operator<(linklayer_address const & lhs, linklayer_address const & rhs) noexcept
{
    return lhs.bytes_ < rhs.bytes_;
}

inline std::size_t hash_value(linklayer_address const & rhs) noexcept
{
    return detail::address_impl<linklayer_address>::template hash_func<std::size_t>(rhs.bytes_.data());
}

template <typename Ch, typename Tr>
inline std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs)
{
    char buf[18];
    detail::address_impl<linklayer_address>::to_string(rhs.bytes_, buf);
    std::copy_n(buf, 17, std::ostreambuf_iterator<Ch>(os));
    return os;
}

} }
