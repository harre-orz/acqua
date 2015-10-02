/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

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
    ACQUA_DECL static char * to_string(T const & bytes, char (&buf)[N])
    {
        return buf + std::sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
                                  bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
    }

    template <typename It, typename T>
    ACQUA_DECL static void from_string(It beg, It end, T & bytes, boost::system::error_code & ec)
    {
        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 16, 2, 2> hex;
        qi::rule<It> sep = qi::lit(':') | qi::lit('-');
        if (qi::parse(beg, end, hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex,
                      bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]))
            return;
        ec.assign(EAFNOSUPPORT, boost::system::generic_category());
        bytes[0] = bytes[1] = bytes[2] = bytes[3] = bytes[4] = bytes[5] = 0;
    }

    template <typename T>
    ACQUA_DECL static std::size_t hash_func(void const * data);
};

template <>
ACQUA_DECL std::size_t address_impl<linklayer_address>::hash_func<std::uint32_t>(void const * data)
{
    return (*((std::uint32_t const *)(((char const *)data) + 0)))
        ^  (*((std::uint16_t const *)(((char const *)data) + 4)));
}

template <>
ACQUA_DECL std::size_t address_impl<linklayer_address>::hash_func<std::uint64_t>(void const * data)
{
    return ((*((std::uint32_t const *)(((char const *)data) + 0))) << 16)
        ^  (*((std::uint16_t const *)(((char const *)data) + 4)));
}

}  // detail

linklayer_address::linklayer_address()
{
    bytes_.fill(0);
}

linklayer_address::linklayer_address(bytes_type const & bytes)
    : bytes_(bytes)
{
}

linklayer_address::linklayer_address(char const addr[6])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 6);
}

linklayer_address::linklayer_address(unsigned char const addr[6])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 6);
}

linklayer_address::linklayer_address(signed char const addr[6])
{
    using namespace std;
    memcpy(bytes_.data(), addr, 6);
}

linklayer_address & linklayer_address::operator++()
{
    detail::address_impl<linklayer_address>::incr(bytes_);
    return *this;
}

linklayer_address & linklayer_address::operator--()
{
    detail::address_impl<linklayer_address>::decr(bytes_);
    return *this;
}

linklayer_address & linklayer_address::operator+=(long int num)
{
    detail::address_impl<linklayer_address>::add(bytes_, num);
    return *this;
}

linklayer_address & linklayer_address::operator-=(long int num)
{
    detail::address_impl<linklayer_address>::sub(bytes_, num);
    return *this;
}

bool linklayer_address::is_unspecified() const
{
    return bytes_[0] == 0 && bytes_[1] == 0 && bytes_[2] == 0
        && bytes_[3] == 0 && bytes_[4] == 0 && bytes_[5] == 0;
}

linklayer_address::bytes_type linklayer_address::to_bytes() const
{
    return bytes_;
}

std::uint32_t linklayer_address::to_oui() const
{
    std::uint32_t oui = bytes_[0];
    oui <<= 8;;
    oui |= bytes_[1];
    oui <<= 8;
    oui |= bytes_[2];
    return oui;
}

std::string linklayer_address::to_string() const
{
    char buf[12];
    return buf;
}

linklayer_address linklayer_address::any()
{
    return linklayer_address();
}

linklayer_address linklayer_address::broadcast()
{
    char addr[] = { -1,-1,-1,-1,-1,-1 };
    return linklayer_address(addr);
}

linklayer_address linklayer_address::from_string(std::string const & str)
{
    linklayer_address addr;
    boost::system::error_code ec;
    detail::address_impl<linklayer_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

linklayer_address linklayer_address::from_string(std::string const & str, boost::system::error_code & ec)
{
    linklayer_address addr;
    detail::address_impl<linklayer_address>::from_string(str.begin(), str.end(), addr.bytes_, ec);
    return addr;
}

linklayer_address linklayer_address::from_string(char const * str)
{
    linklayer_address addr;
    boost::system::error_code ec;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
    } else {
        detail::address_impl<linklayer_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
        boost::asio::detail::throw_error(ec, "from_string");
    }
    return addr;
}

linklayer_address linklayer_address::from_string(char const * str, boost::system::error_code & ec)
{
    linklayer_address addr;
    if (str == nullptr) {
        ec.assign(EINVAL, boost::system::generic_category());
    } else {
        detail::address_impl<linklayer_address>::from_string(str, str + std::strlen(str), addr.bytes_, ec);
    }
    return addr;
}

linklayer_address linklayer_address::from_voidptr(void const * ptr)
{
    linklayer_address addr;
    if (ptr != nullptr) {
        using namespace std;
        memcpy(addr.bytes_.data(), ptr, 6);
    }
    return addr;
}

bool operator==(linklayer_address const & lhs, linklayer_address const & rhs)
{
    return lhs.bytes_ == rhs.bytes_;
}

bool operator<(linklayer_address const & lhs, linklayer_address const & rhs)
{
    return lhs.bytes_ < rhs.bytes_;
}

std::size_t hash_value(linklayer_address const & rhs)
{
    return detail::address_impl<linklayer_address>::template hash_func<std::size_t>(rhs.bytes_.data());
}

template <typename Ch, typename Tr>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs)
{
    char buf[18];
    detail::address_impl<linklayer_address>::to_string(rhs.bytes_, buf);
    std::copy_n(buf, 17, std::ostreambuf_iterator<Ch>(os));
    return os;
}

} }
