/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

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
    using masklen_type = unsigned char;

    ACQUA_DECL internet6_address();
    ACQUA_DECL internet6_address(internet6_address const &) = default;

    ACQUA_DECL internet6_address(internet6_address &&) = default;

    ACQUA_DECL internet6_address(bytes_type const & bytes);

    ACQUA_DECL explicit internet6_address(char const addr[16]);

    ACQUA_DECL explicit internet6_address(signed char const addr[16]);

    ACQUA_DECL explicit internet6_address(unsigned char const addr[16]);

    ACQUA_DECL internet6_address(struct ::in6_addr const & addr);

    ACQUA_DECL internet6_address(boost::asio::ip::address_v6 const & rhs);

    ACQUA_DECL explicit internet6_address(std::uint64_t low);

    ACQUA_DECL explicit internet6_address(std::uint64_t high, std::uint64_t low);

    ACQUA_DECL internet6_address & operator=(internet6_address const &) = default;

    ACQUA_DECL internet6_address & operator=(internet6_address &&) = default;

    ACQUA_DECL internet6_address & operator++();

    ACQUA_DECL internet6_address & operator--();

    ACQUA_DECL internet6_address & operator+=(long int num);

    ACQUA_DECL internet6_address & operator-=(long int num);

    ACQUA_DECL operator ::in6_addr() const;

    ACQUA_DECL operator boost::asio::ip::address_v6() const;

    ACQUA_DECL bool is_unspecified() const;

    ACQUA_DECL bool is_loopback() const;

    ACQUA_DECL bool is_link_local() const;

    ACQUA_DECL bool is_site_local() const;

    ACQUA_DECL bool is_v4_mapped() const;

    ACQUA_DECL bool is_v4_compatible() const;

    ACQUA_DECL bool is_multicast() const;

    ACQUA_DECL bool is_multicast_global() const;

    ACQUA_DECL bool is_multicast_link_local() const;

    ACQUA_DECL bool is_multicast_node_local() const;

    ACQUA_DECL bool is_multicast_org_local() const;

    ACQUA_DECL bool is_multicast_site_local() const;

    ACQUA_DECL bool is_netmask() const;

    ACQUA_DECL bytes_type to_bytes() const;

    ACQUA_DECL std::string to_string() const;

    ACQUA_DECL static internet6_address any()
    {
        return internet6_address();
    }

    ACQUA_DECL static internet6_address loopback()
    {
        return internet6_address(1);
    }

    ACQUA_DECL static internet6_address from_string(std::string const & str);

    ACQUA_DECL static internet6_address from_string(std::string const & str, boost::system::error_code & ec);

    ACQUA_DECL static internet6_address from_string(char const * str);

    ACQUA_DECL static internet6_address from_string(char const * str, boost::system::error_code & ec);

    ACQUA_DECL static internet6_address from_voidptr(void const * ptr);

    ACQUA_DECL friend bool operator==(internet6_address const & lhs, internet6_address const & rhs);

    ACQUA_DECL friend bool operator==(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs);

    ACQUA_DECL friend bool operator<(internet6_address const & lhs, internet6_address const & rhs);

    ACQUA_DECL friend bool operator<(internet6_address const & lhs, boost::asio::ip::address_v6 const & rhs);

    template <typename Ch, typename Tr>
    ACQUA_DECL friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet6_address const & rhs);

    ACQUA_DECL friend std::size_t hash_value(internet6_address const & rhs);

    ACQUA_DECL friend masklen_type netmask_length(internet6_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/internet6_address.ipp>
