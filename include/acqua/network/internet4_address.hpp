/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

#include <iostream>
#include <boost/operators.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/endian/arithmetic.hpp>
#include <acqua/network/basic_prefix_address.hpp>


namespace acqua { namespace network {

/*!
  IPv4アドレスクラス.

  ネットワークバイトオーダー
  boost::asio::ip::address_v4 と互換性あり
 */
class internet4_address
    : private boost::totally_ordered<internet4_address>
    , private boost::totally_ordered<internet4_address, boost::asio::ip::address_v4>
    , private boost::unit_steppable<internet4_address>
    , private boost::additive2<internet4_address, long int>
{
    friend basic_prefix_address<internet4_address>;

public:
    using bytes_type = boost::asio::ip::address_v4::bytes_type;
    using masklen_type = unsigned char;

    ACQUA_DECL internet4_address();

    ACQUA_DECL internet4_address(internet4_address const &) = default;

    ACQUA_DECL internet4_address(internet4_address &&) = default;

    ACQUA_DECL internet4_address(bytes_type const & bytes);

    ACQUA_DECL explicit internet4_address(char const addr[4]);

    ACQUA_DECL explicit internet4_address(signed char const addr[4]);

    ACQUA_DECL explicit internet4_address(unsigned char const addr[4]);

    ACQUA_DECL internet4_address(struct ::in_addr const & addr);

    ACQUA_DECL internet4_address(boost::asio::ip::address_v4 const & addr);

    ACQUA_DECL explicit internet4_address(std::uint32_t addr);

    ACQUA_DECL internet4_address & operator=(internet4_address const &) = default;

    ACQUA_DECL internet4_address & operator=(internet4_address &&) = default;

    ACQUA_DECL internet4_address & operator++();

    ACQUA_DECL internet4_address & operator+=(long int num);

    ACQUA_DECL internet4_address & operator--();

    ACQUA_DECL internet4_address & operator-=(long int num);

    ACQUA_DECL operator ::in_addr() const;

    ACQUA_DECL operator boost::asio::ip::address_v4() const;

    ACQUA_DECL bool is_unspecified() const;

    ACQUA_DECL bool is_loopback() const;

    ACQUA_DECL bool is_class_a() const;

    ACQUA_DECL bool is_class_b() const;

    ACQUA_DECL bool is_class_c() const;

    ACQUA_DECL bool is_multicast() const;

    ACQUA_DECL bool is_link_local() const;

    ACQUA_DECL bool is_netmask() const;

    ACQUA_DECL bytes_type to_bytes() const;

    ACQUA_DECL std::string to_string() const;

    ACQUA_DECL std::uint32_t to_ulong() const;

    ACQUA_DECL static internet4_address any()
    {
        return internet4_address();
    }

    ACQUA_DECL static internet4_address broadcast()
    {
        return internet4_address(0xFFFFFFFF);
    }

    ACQUA_DECL static internet4_address loopback()
    {
        return internet4_address(0x7F000001);
    }

    ACQUA_DECL static internet4_address from_string(std::string const & str);

    ACQUA_DECL static internet4_address from_string(std::string const & str, boost::system::error_code & ec);

    ACQUA_DECL static internet4_address from_string(char const * str);

    ACQUA_DECL static internet4_address from_string(char const * str, boost::system::error_code & ec);

    ACQUA_DECL friend bool operator==(internet4_address const & lhs, internet4_address const & rhs);

    ACQUA_DECL friend bool operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs);

    ACQUA_DECL friend bool operator<(internet4_address const & lhs, internet4_address const & rhs);

    ACQUA_DECL friend bool operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs);

    template <typename Ch, typename Tr>
    ACQUA_DECL friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs);

    ACQUA_DECL friend std::size_t hash_value(internet4_address const & rhs);

    ACQUA_DECL friend int netmask_length(internet4_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/internet4_address.ipp>
