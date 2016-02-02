/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <boost/system/error_code.hpp>
#include <boost/operators.hpp>
#include <boost/array.hpp>
#include <iostream>

namespace acqua { namespace network {

/*!
  リンクレイヤーアドレス.
*/
class linklayer_address
    : boost::totally_ordered<linklayer_address>
    , boost::unit_steppable<linklayer_address>
    , boost::additive2<linklayer_address, long int>
{
public:
    using bytes_type = boost::array<unsigned char, 6>;

public:
    constexpr linklayer_address() noexcept;

    constexpr linklayer_address(bytes_type const & bytes) noexcept;

    constexpr linklayer_address(linklayer_address const & rhs) noexcept = default;

    constexpr linklayer_address(linklayer_address && rhs) noexcept = default;

    linklayer_address & operator=(linklayer_address const & rhs) noexcept = default;

    linklayer_address & operator=(linklayer_address && rhs) noexcept = default;

    linklayer_address & operator++() noexcept;

    linklayer_address & operator--() noexcept;

    linklayer_address & operator+=(long int num) noexcept;

    linklayer_address & operator-=(long int num) noexcept;

    bool is_unspecified() const noexcept;

    constexpr bytes_type to_bytes() const noexcept;

    std::uint32_t to_oui() const noexcept;

    std::string to_string() const;

    static constexpr linklayer_address any() noexcept;

    static constexpr linklayer_address broadcast() noexcept;

    static linklayer_address from_string(std::string const & str);

    static linklayer_address from_string(std::string const & str, boost::system::error_code & ec) noexcept;

    static linklayer_address from_string(char const * str);

    static linklayer_address from_string(char const * str, boost::system::error_code & ec) noexcept;

    static linklayer_address from_voidptr(void const * ptr) noexcept;

    friend bool operator==(linklayer_address const & lhs, linklayer_address const & rhs) noexcept;

    friend bool operator<(linklayer_address const & lhs, linklayer_address const & rhs) noexcept;

    friend std::size_t hash_value(linklayer_address const & rhs) noexcept;

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/linklayer_address.ipp>
