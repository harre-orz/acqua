/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <iostream>
#include <boost/operators.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/endian/arithmetic.hpp>
#include <acqua/network/basic_prefix_address.hpp>


namespace acqua { namespace network {

/*!
  IPv4アドレスクラス.

  ネットワークバイトオーダーの in_addr と同等のデータ構造を持つ
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

    internet4_address();

    internet4_address(internet4_address const & rhs);

    internet4_address(internet4_address && rhs);

    internet4_address(bytes_type const & bytes);

    explicit internet4_address(char const addr[4]);

    explicit internet4_address(signed char const addr[4]);

    explicit internet4_address(unsigned char const addr[4]);

    internet4_address(struct ::in_addr const & addr);

    internet4_address(boost::asio::ip::address_v4 const & addr);

    explicit internet4_address(std::uint32_t addr);

    internet4_address & operator=(internet4_address const & rhs);

    internet4_address & operator=(internet4_address && rhs);

    internet4_address & operator++();

    internet4_address & operator+=(long int num);

    internet4_address & operator--();

    internet4_address & operator-=(long int num);

    operator ::in_addr() const;

    operator boost::asio::ip::address_v4() const;

    bool is_unspecified() const;

    bool is_loopback() const;

    bool is_class_a() const;

    bool is_class_b() const;

    bool is_class_c() const;

    bool is_multicast() const;

    bool is_link_local() const;

    bool is_netmask() const;

    bytes_type to_bytes() const;

    std::string to_string() const;

    std::uint32_t to_ulong() const;

    static internet4_address any()
    {
        return internet4_address();
    }

    static internet4_address broadcast()
    {
        return internet4_address(0xFFFFFFFF);
    }

    static internet4_address loopback()
    {
        return internet4_address(0x7F000001);
    }

    static internet4_address from_string(std::string const & str);

    static internet4_address from_string(std::string const & str, boost::system::error_code & ec);

    static internet4_address from_string(char const * str);

    static internet4_address from_string(char const * str, boost::system::error_code & ec);

    friend bool operator==(internet4_address const & lhs, internet4_address const & rhs);

    friend bool operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs);

    friend bool operator<(internet4_address const & lhs, internet4_address const & rhs);

    friend bool operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs);

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs);

    friend std::size_t hash_value(internet4_address const & rhs);

    friend int netmask_length(internet4_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/internet4_address.ipp>
