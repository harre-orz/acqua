/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
}

#include <iostream>
#include <type_traits>
#include <boost/operators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace network {

/*!
  IPv4アドレスクラス.

  trivial なデータ型
  boost::asio::ip::address_v4 と互換性あり
 */
class internet4_address
    : private boost::totally_ordered<internet4_address>
    , private boost::totally_ordered<internet4_address, boost::asio::ip::address_v4>
    , private boost::unit_steppable<internet4_address>
    , private boost::additive2<internet4_address, long int>
{
public:
    using bytes_type = boost::asio::ip::address_v4::bytes_type;

    internet4_address() noexcept
    {
        static_assert(sizeof(*this) == 4, "");
        bytes_.fill(0);
    }

    internet4_address(internet4_address const &) noexcept = default;

    internet4_address(internet4_address &&) noexcept= default;

    internet4_address(bytes_type const & bytes) noexcept
        : bytes_(bytes)
    {
    }

    internet4_address(char const addr[4]) noexcept
    {
        copy_from(addr);
    }

    internet4_address(unsigned char addr[4]) noexcept
    {
        copy_from(addr);
    }

    internet4_address(signed char addr[4]) noexcept
    {
        copy_from(addr);
    }

    internet4_address(struct ::in_addr const & addr) noexcept
    {
        copy_from(&addr.s_addr);
    }

    internet4_address(std::uint32_t addr) noexcept
    {
        auto num = ntohl(addr);
        copy_from(&num);
    }

    internet4_address(boost::asio::ip::address_v4 const & addr) noexcept
        : bytes_(addr.to_bytes())
    {
    }

    internet4_address & operator=(internet4_address const &) = default;

    internet4_address & operator=(internet4_address &&) = default;

    bool operator==(internet4_address const & rhs) const noexcept
    {
        return bytes_ == rhs.bytes_;
    }

    bool operator==(boost::asio::ip::address_v4 const & rhs) const noexcept
    {
        return bytes_ == rhs.to_bytes();
    }

    bool operator<(internet4_address const & rhs) const noexcept
    {
        return bytes_ < rhs.bytes_;
    }

    internet4_address & operator++() noexcept
    {
        for(auto it = bytes_.rbegin(); ++(*it) == 0x00 && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    internet4_address & operator+=(long int num) noexcept
    {
        return (*this = internet4_address(to_ulong() + num));
    }

    internet4_address & operator--() noexcept
    {
        for(auto it = bytes_.rbegin(); --(*it) == 0xff && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    internet4_address & operator-=(long int num) noexcept
    {
        return (*this = internet4_address(to_ulong() - num));
    }

    static internet4_address any() noexcept
    {
        return internet4_address();
    }

    static internet4_address broadcast() noexcept
    {
        bytes_type bytes;
        bytes.fill(255);
        return internet4_address(bytes);
    }

    static internet4_address loopback()
    {
        bytes_type bytes({ 127,0,0,1 });
        return internet4_address(bytes);
    }

    static internet4_address from_string(std::string const & str, boost::system::error_code & ec) noexcept
    {
        return from_string(str.begin(), str.end(), ec);
    }

    static internet4_address from_string(std::string const & str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str.begin(), str.end(), ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    static internet4_address from_string(char const * str, boost::system::error_code & ec) noexcept
    {
        return from_string(str, str + std::strlen(str), ec);
    }

    static internet4_address from_string(char const * str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str, str + std::strlen(str), ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    bool is_class_a() const noexcept
    {
        return (bytes_[0] & 0x80) == 0x00;
    }

    bool is_class_b() const noexcept
    {
        return (bytes_[0] & 0xc0) == 0x80;
    }

    bool is_class_c() const noexcept
    {
        return (bytes_[0] & 0xe0) == 0xc0;
    }

    bool is_unspecified() const noexcept
    {
        return *this == any();
    }

    bool is_loopback() const noexcept
    {
        return *this == loopback();
    }

    bool is_multicast() const noexcept
    {
        return (bytes_[0] & 0xfe) == 0xfe;
    }

    bool is_link_local() const noexcept
    {
        return (bytes_[0] & 0x169 && bytes_[1] == 254);
    }

    bool is_netmask() const noexcept
    {
        auto it = bytes_.begin();
        while(it != bytes_.end()) {
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
                    return std::accumulate(it, bytes_.end(), 0) == 0;
                default:
                    return false;
            }
        }
        return true;
    }

    bytes_type to_bytes() const noexcept
    {
        return bytes_;
    }

    std::string to_string() const
    {
        return boost::lexical_cast<std::string>(*this);
    }

    std::uint32_t to_ulong() const noexcept
    {
        return ntohl(*reinterpret_cast<std::uint32_t const *>(bytes_.data()));
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs)
    {
        char buf[4*4];
        auto size = std::sprintf(buf, "%d.%d.%d.%d", rhs.bytes_[0], rhs.bytes_[1], rhs.bytes_[2], rhs.bytes_[3]);
        std::copy_n(buf, size, std::ostreambuf_iterator<Ch>(os));
        return os;
    }

    friend std::size_t hash_value(internet4_address const & rhs)
    {
        return *reinterpret_cast<std::uint32_t const *>(rhs.bytes_.data());
    }

private:
    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    void copy_from(T const * t) noexcept
    {
        reinterpret_cast<std::uint32_t *>(bytes_.data())[0] = reinterpret_cast<std::uint32_t const *>(t)[0];
    }

    /*!
      RFC3986 準拠
      IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet

      dec-octet   = DIGIT                 ; 0-9
                   / %x31-39 DIGIT         ; 10-99
                   / "1" 2DIGIT            ; 100-199
                   / "2" %x30-34 DIGIT     ; 200-249
                   / "25" %x30-35          ; 250-255
    */
    template <typename It>
    static internet4_address from_string(It beg, It end, boost::system::error_code & ec)
    {
        bytes_type bytes;

        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 10, 1, 3> dec;

        if (!qi::parse(beg, end, dec >> '.' >> dec >> '.' >> dec >> '.' >> dec,
                       bytes[0], bytes[1], bytes[2], bytes[3]) || beg != end)
            ec.assign(EAFNOSUPPORT, boost::system::generic_category());

        return internet4_address(bytes);
    }

private:
    bytes_type bytes_;
} __attribute__((__packed__));

} }
