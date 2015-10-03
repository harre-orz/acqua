/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/basic_prefix_address.hpp>
#include <acqua/network/detail/address_impl.hpp>

namespace acqua { namespace network {

class internet4_address;
class internet6_address;

namespace detail {

template <typename T> ACQUA_DECL std::size_t max_masklen();
template <> ACQUA_DECL std::size_t max_masklen<internet4_address>() { return  32; }
template <> ACQUA_DECL std::size_t max_masklen<internet6_address>() { return 128; }

}  // detail

template <typename T>
basic_prefix_address<T>::basic_prefix_address()
    : masklen_(0)
{
}

template <typename T>
basic_prefix_address<T>::basic_prefix_address(address_type const & address, masklen_type masklen)
{
    assign(address, masklen);
}


template <typename T>
void basic_prefix_address<T>::assign(address_type const & address, masklen_type masklen)
{
    masklen_ = std::min<masklen_type>(masklen, detail::max_masklen<T>());
    address_ = address_type::any();

    auto iit = address.bytes_.begin();
    auto oit = address_.bytes_.begin();
    for(masklen = masklen_; masklen > 0; masklen -= 8)
        *oit++ = *iit++;
    *oit = *iit & ~((0x01 << (8 - masklen)) - 1);
}


template <typename T>
T basic_prefix_address<T>::netmask() const
{
    address_type addr;
    auto it = addr.bytes_.begin();
    for(masklen_type len = masklen_ / 8; len; --len)
        *it++ = 0xFF;
    *it = ~((0x01 << (8 - masklen_ % 8)) - 1);
    return addr;
}


template <typename T>
basic_prefix_address<T> & basic_prefix_address<T>::operator++()
{
    address_ += (1 << (detail::max_masklen<T>() - masklen_));
    return *this;
}


template <typename T>
basic_prefix_address<T> & basic_prefix_address<T>::operator--()
{
    address_ -= (1 << (detail::max_masklen<T>() - masklen_));
    return *this;
}


template <typename T>
basic_prefix_address<T> & basic_prefix_address<T>::operator+=(long int num)
{
    if (num < 0)
        return operator-=(-num);
    uint i = 1;
    uint len = detail::max_masklen<T>() - masklen_;
    while(num) {
        address_ += (i << len);
        num -= i;
        i <<= 1;
    }
    return *this;
}


template <typename T>
basic_prefix_address<T> & basic_prefix_address<T>::operator-=(long int num)
{
    if (num < 0)
        return operator+=(-num);
    uint i = 1;
    uint len = detail::max_masklen<T>() - masklen_;
    while(num) {
        address_ -= (i << len);
        num -= i;
        i <<= 1;
    }
    return *this;
}


template <typename T>
bool operator==(basic_prefix_address<T> const & lhs, basic_prefix_address<T> const & rhs)
{
    return lhs.masklen_ == rhs.masklen_ && lhs.address_ == rhs.address_;
}


template <typename T>
bool operator<(basic_prefix_address<T> const & lhs, basic_prefix_address<T> const & rhs)
{
    return lhs.masklen_ < rhs.masklen_ || lhs.address_ < rhs.address_;
}

template <typename T> template <typename Ch, typename Tr>
void basic_prefix_address<T>::to_string(std::basic_ostream<Ch, Tr> & os) const
{
    char buf[64];
    char * end = detail::address_impl<T>::to_string(address_.bytes_, buf);
    *end++ = '/';
    if ((*end = '0' + masklen_ / 100 % 10) != '0') end++;
    if ((*end = '0' + masklen_ /  10 % 10) != '0') end++;
    *end++ = '0' + masklen_ % 10;
    std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
}

template <typename T, typename Ch, typename Tr>
std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_prefix_address<T> const & rhs)
{
    rhs.to_string(os);
    return os;
}

} }
