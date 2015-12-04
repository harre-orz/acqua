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
    using masklen_type = std::uint16_t;

public:
    constexpr internet4_address() noexcept;

    constexpr internet4_address(bytes_type const & bytes) noexcept;

    constexpr internet4_address(std::uint8_t a, std::uint8_t b, std::uint8_t c, std::uint8_t d) noexcept;

    internet4_address(struct ::in_addr const & addr) noexcept;

    internet4_address(boost::asio::ip::address_v4 const & addr) noexcept;

    explicit internet4_address(std::uint32_t addr) noexcept;

    constexpr internet4_address(internet4_address const & rhs) noexcept = default;

    constexpr internet4_address(internet4_address && rhs) noexcept = default;

    internet4_address & operator=(internet4_address const & rhs) noexcept = default;

    internet4_address & operator=(internet4_address && rhs) noexcept = default;

    internet4_address & operator++() noexcept;

    internet4_address & operator+=(long int num) noexcept;

    internet4_address & operator--() noexcept;

    internet4_address & operator-=(long int num) noexcept;

    operator ::in_addr() const noexcept;

    operator boost::asio::ip::address_v4() const noexcept;

    bool is_unspecified() const noexcept;

    bool is_loopback() const noexcept;

    bool is_class_a() const noexcept;

    bool is_class_b() const noexcept;

    bool is_class_c() const noexcept;

    bool is_multicast() const noexcept;

    bool is_link_local() const noexcept;

    bool is_netmask() const noexcept;

    constexpr bytes_type to_bytes() const noexcept;

    std::string to_string() const;

    std::uint32_t to_ulong() const noexcept;

    void checksum(std::size_t & sum) const noexcept;

    static constexpr internet4_address any() noexcept;

    static constexpr internet4_address broadcast() noexcept;

    static constexpr internet4_address loopback() noexcept;

    static internet4_address from_string(std::string const & str);

    static internet4_address from_string(std::string const & str, boost::system::error_code & ec) noexcept;

    static internet4_address from_string(char const * str);

    static internet4_address from_string(char const * str, boost::system::error_code & ec) noexcept;

    static internet4_address from_voidptr(void const * ptr) noexcept;

    friend bool operator==(internet4_address const & lhs, internet4_address const & rhs) noexcept;

    friend bool operator==(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) noexcept;

    friend bool operator<(internet4_address const & lhs, internet4_address const & rhs) noexcept;

    friend bool operator<(internet4_address const & lhs, boost::asio::ip::address_v4 const & rhs) noexcept;

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, internet4_address const & rhs);

    friend std::size_t hash_value(internet4_address const & rhs) noexcept;

    friend int netmask_length(internet4_address const & rhs) noexcept;

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/internet4_address.ipp>
