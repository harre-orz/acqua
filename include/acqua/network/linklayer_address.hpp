/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

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

    ACQUA_DECL linklayer_address();

    ACQUA_DECL linklayer_address(linklayer_address const &) = default;

    ACQUA_DECL linklayer_address(linklayer_address &&) = default;

    ACQUA_DECL linklayer_address(bytes_type const & bytes);

    ACQUA_DECL linklayer_address(char const addr[6]);

    ACQUA_DECL linklayer_address(unsigned char const addr[6]);

    ACQUA_DECL linklayer_address(signed char const addr[6]);

    ACQUA_DECL linklayer_address & operator=(linklayer_address const &) = default;

    ACQUA_DECL linklayer_address & operator=(linklayer_address &&) = default;

    ACQUA_DECL linklayer_address & operator++();

    ACQUA_DECL linklayer_address & operator--();

    ACQUA_DECL linklayer_address & operator+=(long int num);

    ACQUA_DECL linklayer_address & operator-=(long int num);

    ACQUA_DECL bool is_unspecified() const;

    ACQUA_DECL bytes_type to_bytes() const;

    ACQUA_DECL std::uint32_t to_oui() const;

    ACQUA_DECL std::string to_string() const;

    ACQUA_DECL static linklayer_address any();

    ACQUA_DECL static linklayer_address broadcast();

    ACQUA_DECL static linklayer_address from_string(std::string const & str);

    ACQUA_DECL static linklayer_address from_string(std::string const & str, boost::system::error_code & ec);

    ACQUA_DECL static linklayer_address from_string(char const * str);

    ACQUA_DECL static linklayer_address from_string(char const * str, boost::system::error_code & ec);

    ACQUA_DECL static linklayer_address from_voidptr(void const * bytes);

    ACQUA_DECL friend bool operator==(linklayer_address const & lhs, linklayer_address const & rhs);

    ACQUA_DECL friend bool operator<(linklayer_address const & lhs, linklayer_address const & rhs);

    ACQUA_DECL friend std::size_t hash_value(linklayer_address const & rhs);

    template <typename Ch, typename Tr>
    ACQUA_DECL friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs);

private:
    bytes_type bytes_;
};

} }

#include <acqua/network/impl/linklayer_address.ipp>
