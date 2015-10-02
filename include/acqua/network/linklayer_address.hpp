/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <boost/array.hpp>
#include <boost/operators.hpp>
#include <boost/system/error_code.hpp>

namespace acqua { namespace network {

/*!
  リンクレイヤーアドレス.

  trivial なデータ型
 */
class linklayer_address
    : boost::totally_ordered<linklayer_address>
    , boost::unit_steppable<linklayer_address>
    , boost::additive2<linklayer_address, long int>
{
public:
    using bytes_type = boost::array<unsigned char, 6>;

    linklayer_address();

    linklayer_address(linklayer_address const &) = default;

    linklayer_address(linklayer_address &&) = default;

    linklayer_address(bytes_type const & bytes);

    linklayer_address(char const addr[6]);

    linklayer_address(unsigned char const addr[6]);

    linklayer_address(signed char const addr[6]);

    linklayer_address & operator=(linklayer_address const &) = default;

    linklayer_address & operator=(linklayer_address &&) = default;

    linklayer_address & operator++();

    linklayer_address & operator--();

    linklayer_address & operator+=(long int num);

    linklayer_address & operator-=(long int num);

    bool is_unspecified() const;

    bytes_type to_bytes() const;

    std::uint32_t to_oui() const;

    std::string to_string() const;

    static linklayer_address any();

    static linklayer_address broadcast();

    static linklayer_address from_string(std::string const & str);

    static linklayer_address from_string(std::string const & str, boost::system::error_code & ec);

    static linklayer_address from_string(char const * str);

    static linklayer_address from_string(char const * str, boost::system::error_code & ec);

    static linklayer_address from_voidptr(void const * bytes);

    friend bool operator==(linklayer_address const & lhs, linklayer_address const & rhs);

    friend bool operator<(linklayer_address const & lhs, linklayer_address const & rhs);

    friend std::size_t hash_value(linklayer_address const & rhs);

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/linklayer_address.ipp>
