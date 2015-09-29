/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/spirit/include/qi.hpp>

namespace acqua { namespace network {

namespace detail {

template <typename It>
void from_string(It beg, It end, internet4_address::bytes_type & bytes, boost::system::error_code & ec)
{
    namespace qi = boost::spirit::qi;
    qi::uint_parser<unsigned char, 10, 1, 3> dec;

    if (qi::parse(beg, end, dec >> '.' >> dec >> '.' >> dec >> '.' >> dec,
                   bytes[0], bytes[1], bytes[2], bytes[3]))
        return;
    ec.assign(EAFNOSUPPORT, boost::system::generic_category());
    bytes.fill(0);
}

char * to_string(internet4_address::bytes_type const & bytes, char * buf)
{
    return buf + std::sprintf(buf, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

}  // detail

internet4_address::internet4_address(bytes_type const & bytes)
    : addr_(*reinterpret_cast<decltype(addr_) const *>(bytes.data()))
{
}

internet4_address::internet4_address(char const addr[4])
    : addr_(*reinterpret_cast<decltype(addr_) const *>(addr))
{
}

internet4_address::internet4_address(signed char const addr[4])
    : addr_(*reinterpret_cast<decltype(addr_) const *>(addr))
{
}

internet4_address::internet4_address(unsigned char const addr[4])
    : addr_(*reinterpret_cast<decltype(addr_) const *>(addr))
{
}

internet4_address & internet4_address::operator++()
{
    ++addr_;
    return *this;
}

internet4_address & internet4_address::operator--()
{
    --addr_;
    return *this;
}

internet4_address & internet4_address::operator+=(long int num)
{
    addr_ += num;
    return *this;
}

internet4_address & internet4_address::operator-=(long int num)
{
    addr_ -= num;
    return *this;
}

internet4_address::operator ::in_addr() const
{
    return reinterpret_cast<::in_addr const &>(addr_);
}

internet4_address::operator boost::asio::ip::address_v4() const
{
    return boost::asio::ip::address_v4(to_bytes());
}

bool internet4_address::is_unspecified() const
{
    return addr_ == 0;
}

bool internet4_address::is_loopback() const
{
    return (addr_ & 0xFF000000) == 0x7F000000;
}

bool internet4_address::is_class_a() const
{
    return (addr_ & 0x80000000) == 0;
}

bool internet4_address::is_class_b() const
{
    return (addr_ & 0xC0000000) == 0x80000000;
}

bool internet4_address::is_class_c() const
{
    return (addr_ & 0xE0000000) == 0xC0000000;
}

bool internet4_address::is_multicast() const
{
    return (addr_ & 0xF0000000) == 0xE0000000;
}

bool internet4_address::is_link_local() const
{
    return (addr_ & 0xFFFF0000) == 0xA9FE0000;
}

bool internet4_address::is_netmask() const
{
    auto bytes = reinterpret_cast<bytes_type const *>(&addr_);
    for(auto it = bytes->begin(), end = bytes->end(); it != end; ++it) {
        switch(*it) {
            case 0b11111111:
                ;
            case 0b10000000:
            case 0b11000000:
            case 0b11100000:
            case 0b11110000:
            case 0b11111000:
            case 0b11111100:
            case 0b11111110:
                return std::accumulate(it, end, 0) == 0;
            default:
                return false;
        }
    }
    return true;
}

internet4_address::bytes_type internet4_address::to_bytes() const
{
    return *reinterpret_cast<bytes_type const *>(&addr_);
}

std::string internet4_address::to_string() const
{
    char buf[4*4];
    detail::to_string(reinterpret_cast<bytes_type const &>(addr_), buf);
    return buf;
}

std::uint32_t internet4_address::to_ulong() const
{
    return addr_;
}

internet4_address internet4_address::from_string(std::string const & str)
{
    boost::system::error_code ec;
    internet4_address addr;
    detail::from_string(str.begin(), str.end(), reinterpret_cast<bytes_type &>(addr.addr_), ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

internet4_address internet4_address::from_string(std::string const & str, boost::system::error_code & ec)
{
    internet4_address addr;
    detail::from_string(str.begin(), str.end(), reinterpret_cast<bytes_type &>(addr.addr_), ec);
    return addr;
}

internet4_address internet4_address::from_string(char const * str)
{
    boost::system::error_code ec;
    internet4_address addr;
    detail::from_string(str, str + std::strlen(str), reinterpret_cast<bytes_type &>(addr.addr_), ec);
    boost::asio::detail::throw_error(ec, "from_string");
    return addr;
}

internet4_address internet4_address::from_string(char const * str, boost::system::error_code & ec)
{
    internet4_address addr;
    detail::from_string(str, str + std::strlen(str), reinterpret_cast<bytes_type &>(addr.addr_), ec);
    return addr;
}

std::size_t hash_value(internet4_address const & rhs)
{
    return rhs.addr_;
}

bool operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs)
{
    return lhs.addr_ == rhs.to_ulong();
}

bool operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs)
{
    return lhs.addr_ < rhs.to_ulong();
}

template <typename Ch, typename Tr>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs)
{
    char buf[4*4];
    char * end = detail::to_string(reinterpret_cast<internet4_address::bytes_type const &>(rhs.addr_), buf);
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    return os;
}

} }
