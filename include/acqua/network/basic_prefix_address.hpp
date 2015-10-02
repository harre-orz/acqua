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

namespace acqua { namespace network {

template <typename T>
class basic_prefix_address
    : private boost::totally_ordered< basic_prefix_address<T> >
{
public:
    using address_type = T;
    using masklen_type = typename T::masklen_type;

public:
    ACQUA_DECL basic_prefix_address();

    ACQUA_DECL basic_prefix_address(basic_prefix_address const &) = default;

    ACQUA_DECL basic_prefix_address(basic_prefix_address &&) = default;

    ACQUA_DECL explicit basic_prefix_address(address_type const & address, masklen_type masklen);

    ACQUA_DECL void assign(address_type const & address, masklen_type masklen);

    ACQUA_DECL masklen_type masklen() const
    {
        return masklen_;
    }

    ACQUA_DECL address_type address() const
    {
        return address_;
    }

    ACQUA_DECL address_type netmask() const;

    ACQUA_DECL basic_prefix_address & operator=(basic_prefix_address const &) = default;

    ACQUA_DECL basic_prefix_address & operator=(basic_prefix_address &&) = default;

    ACQUA_DECL basic_prefix_address & operator++();

    ACQUA_DECL basic_prefix_address & operator--();

    template <typename T_>
    friend bool operator==(basic_prefix_address<T_> const & lhs, basic_prefix_address<T_> const & rhs);

    template <typename T_>
    friend bool operator<(basic_prefix_address<T_> const & lhs, basic_prefix_address<T_> const & rhs);

    template <typename T_, typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_prefix_address<T_> const & rhs);

    template <typename T_>
    friend std::size_t hash_value(basic_prefix_address<T_> const & rhs);

private:
    masklen_type masklen_;
    address_type address_;
};

} }

#include <acqua/network/impl/basic_prefix_address.ipp>
