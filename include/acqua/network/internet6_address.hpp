/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

extern "C" {
#include <netinet/in.h>
}

#include <iostream>
#include <boost/operators.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <acqua/network/basic_prefix_address.hpp>

namespace acqua { namespace network {

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
    friend basic_prefix_address<internet6_address>;

public:
    using bytes_type = boost::asio::ip::address_v6::bytes_type;
    using masklen_type = std::uint16_t;

    constexpr internet6_address() noexcept;

    constexpr internet6_address(bytes_type const & bytes) noexcept;

    internet6_address(struct ::in6_addr const & addr) noexcept;

    internet6_address(boost::asio::ip::address_v6 const & rhs) noexcept;

    constexpr internet6_address(internet6_address const & rhs) noexcept = default;

    constexpr internet6_address(internet6_address && rhs) noexcept = default;

    internet6_address & operator=(internet6_address const & rhs) noexcept = default;

    internet6_address & operator=(internet6_address && rhs) noexcept = default;

    internet6_address & operator++() noexcept;

    internet6_address & operator--() noexcept;

    internet6_address & operator+=(long int num) noexcept;

    internet6_address & operator-=(long int num) noexcept;

    operator ::in6_addr() const noexcept;

    operator boost::asio::ip::address_v6() const noexcept;

    bool is_unspecified() const noexcept;

    bool is_loopback() const noexcept;

    bool is_link_local() const noexcept;

    bool is_site_local() const noexcept;

    bool is_v4_mapped() const noexcept;

    bool is_v4_compatible() const noexcept;

    bool is_multicast() const noexcept;

    bool is_multicast_global() const noexcept;

    bool is_multicast_link_local() const noexcept;

    bool is_multicast_node_local() const noexcept;

    bool is_multicast_org_local() const noexcept;

    bool is_multicast_site_local() const noexcept;

    bool is_netmask() const noexcept;

    bytes_type to_bytes() const noexcept;

    std::string to_string() const;

    void checksum(std::size_t & sum) const noexcept;

    static constexpr internet6_address any() noexcept
    {
        return internet6_address();
    }

    static constexpr internet6_address loopback() noexcept
    {
        return bytes_type({0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1});
    }

    static internet6_address from_string(std::string const & str);

    static internet6_address from_string(std::string const & str, boost::system::error_code & ec) noexcept;

    static internet6_address from_string(char const * str);

    static internet6_address from_string(char const * str, boost::system::error_code & ec) noexcept;

    static internet6_address from_voidptr(void const * ptr) noexcept;

    friend bool operator==(internet6_address const & lhs, internet6_address const & rhs) noexcept;

    friend bool operator==(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs) noexcept;

    friend bool operator<(internet6_address const & lhs, internet6_address const & rhs) noexcept;

    friend bool operator<(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs) noexcept;

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs);

    friend std::size_t hash_value(internet6_address const & rhs) noexcept;

    friend int netmask_length(internet6_address const & rhs) noexcept;

private:
    bytes_type bytes_;
} __attribute__((__packed__));

} }

#include <acqua/network/impl/internet6_address.ipp>
