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
#include <boost/asio/ip/address_v6.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace network {

namespace detail {

template <typename Address>
class prefix_address;

}


/*!
  IPv6アドレスクラス.

  trivial なデータ型
  boost::asio::ip::address_v5 と互換性あり
 */
class internet6_address
    : private boost::totally_ordered<internet6_address>
    , private boost::totally_ordered<internet6_address, boost::asio::ip::address_v6>
    , private boost::unit_steppable<internet6_address>
    , private boost::additive2<internet6_address, long int>
{
    friend detail::prefix_address<internet6_address>;

public:
    using bytes_type = boost::asio::ip::address_v6::bytes_type;
    using masklen_type = unsigned char;

    internet6_address() noexcept
    {
        static_assert(sizeof(*this) == 16, "");
        bytes_.fill(0);
    }

    internet6_address(internet6_address const &) noexcept = default;
    internet6_address(internet6_address &&) noexcept = default;

    internet6_address(bytes_type const & bytes) noexcept
        : bytes_(bytes)
    {
    }

    internet6_address(char const addr[16]) noexcept
    {
        copy_from(addr);
    }

    internet6_address(unsigned char addr[16]) noexcept
    {
        copy_from(addr);
    }

    internet6_address(signed char addr[16]) noexcept
    {
        copy_from(addr);
    }

    internet6_address(struct ::in6_addr const & addr) noexcept
    {
        copy_from(addr.s6_addr);
    }

    internet6_address & operator=(internet6_address const &) noexcept = default;
    internet6_address & operator=(internet6_address &&) noexcept = default;

    bool operator==(internet6_address const & rhs) const noexcept
    {
        return bytes_ == rhs.bytes_;
    }

    bool operator==(boost::asio::ip::address_v6 const & rhs) const noexcept
    {
        return bytes_ == rhs.to_bytes();
    }

    bool operator<(internet6_address const & rhs) const noexcept
    {
        return bytes_ < rhs.bytes_;
    }

    bool operator<(boost::asio::ip::address_v6 const & rhs) const noexcept
    {
        return bytes_ < rhs.to_bytes();
    }

    internet6_address & operator++() noexcept
    {
        for(auto it = bytes_.rbegin(); ++(*it) == 0x00 && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    internet6_address & operator+=(long int num) noexcept
    {
        if (num < 0)
            return operator-=(-num);

        for(auto it = bytes_.rbegin(); it != bytes_.rend() && num; ++it) {
            *it += (num & 0xff);
            num >>= 8;
        }

        return *this;
    }

    internet6_address & operator--() noexcept
    {
        for(auto it = bytes_.rbegin(); --(*it) == 0xff && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    internet6_address & operator-=(long int num) noexcept
    {
        if (num < 0)
            return operator+=(-num);

        for(auto it = bytes_.rbegin(); it != bytes_.rend() && num; ++it) {
            *it -= (num & 0xff);
            num >>= 8;
        }

        return *this;
    }

    static internet6_address any() noexcept
    {
        return internet6_address();
    }

    static internet6_address loopback() noexcept
    {
        internet6_address addr;
        addr.bytes_[15] = 1;
        return addr;
    }

    static internet6_address from_string(std::string const & str, boost::system::error_code & ec) noexcept
    {
        return from_string(str.c_str(), ec);
    }

    static internet6_address from_string(std::string const & str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str.c_str(), ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    static internet6_address from_string(char const * str, boost::system::error_code & ec) noexcept
    {
        bytes_type bytes;
        if (::inet_pton(AF_INET6, str, bytes.data()) != 1)
            ec.assign(errno, boost::system::generic_category());
        return internet6_address(bytes);
    }

    static internet6_address from_string(char const * str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str, ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    bool is_unspecified() const noexcept
    {
        return *this == any();
    }

    bool is_loopback() const noexcept
    {
        return *this == loopback();
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

    operator ::in6_addr const &() const
    {
        return *reinterpret_cast<::in6_addr const *>(bytes_.data());
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs)
    {
        // char buf[5 * 8];
        // char const * end = rhs.write(buf);
        // std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
        return os;
    }

    friend std::size_t hash_value(internet6_address const & rhs)
    {
        return rhs.hash_func<std::size_t>();
    }

private:
    char * write(char * buf) const
    {
        return const_cast<char *>(::inet_ntop(AF_INET6, bytes_.data(), buf, 40));
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    void copy_from(T const * t) noexcept
    {
        reinterpret_cast<std::uint64_t *>(bytes_.data())[0] = reinterpret_cast<std::uint64_t const *>(t)[0];
        reinterpret_cast<std::uint64_t *>(bytes_.data())[1] = reinterpret_cast<std::uint64_t const *>(t)[1];
        reinterpret_cast<std::uint64_t *>(bytes_.data())[2] = reinterpret_cast<std::uint64_t const *>(t)[2];
        reinterpret_cast<std::uint64_t *>(bytes_.data())[3] = reinterpret_cast<std::uint64_t const *>(t)[3];
    }

    template <typename T>
    T hash_func(typename std::enable_if<sizeof(T) == 4>::type * = nullptr) const noexcept
    {
        return reinterpret_cast<std::uint32_t const *>(bytes_.data())[0]
            ^  reinterpret_cast<std::uint32_t const *>(bytes_.data())[1]
            ^  reinterpret_cast<std::uint32_t const *>(bytes_.data())[2]
            ^  reinterpret_cast<std::uint32_t const *>(bytes_.data())[3];
    }

    template <typename T>
    T hash_func(typename std::enable_if<sizeof(T) == 8>::type * = nullptr) const noexcept
    {
        return reinterpret_cast<std::uint64_t const *>(bytes_.data())[0]
            ^  reinterpret_cast<std::uint64_t const *>(bytes_.data())[1];
    }

private:
    bytes_type bytes_;
} __attribute__((__packed__));

} }
