/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/operators.hpp>

namespace acqua { namespace network {

class internet4_address;
class internet6_address;

namespace detail {

template <typename Address>
class prefix_address
    : private boost::totally_ordered< prefix_address<Address> >
{
    using bytes_type = typename Address::bytes_type;

public:
    using address_type = Address;
    using masklen_type = typename Address::masklen_type;

public:
    prefix_address()
        : masklen_(0), address_() {}

    prefix_address(address_type const & address, masklen_type masklen) noexcept
        : masklen_(std::min(masklen, max_masklen())), address_()
    {
        auto iit = address.bytes_.begin();
        auto oit = address_.bytes_.begin();
        for(masklen = masklen_; masklen > 8; masklen -= 8)
            *oit++ = *iit++;
        *oit = *iit & ~((0x01 << (8 - masklen)) - 1);
    }

    prefix_address(prefix_address const &) = default;
    prefix_address(prefix_address &&) = default;
    prefix_address & operator=(prefix_address const &) = default;
    prefix_address & operator=(prefix_address &&) = default;

    address_type address() const
    {
        return address_;
    }

    address_type netmask() const
    {
        address_type res;
        auto it = res.bytes_.begin();
        for(masklen_type len = masklen_ / 8; len; --len)
            *it++ = 0xff;
        *it = ~((0x01 << (8 - masklen_ % 8)) - 1);
        return res;
    }

    masklen_type masklen() const
    {
        return masklen_;
    }

    prefix_address & operator++()
    {
        auto i = masklen_ / 8 + 1;
        address_.bytes_[i] += (0x01 << (8 - masklen_ % 8));
        while(i > 0 && address_.bytes_[i] == 0x00)
            ++address_.bytes_[--i];
        return *this;
    }

    prefix_address & operator--()
    {
        auto i = masklen_ / 8 + 1;
        auto prev = address_.bytes_[i];
        address_.bytes_[i] -= (0x01 << (8 - masklen_ % 8));
        while(i > 0 && address_.bytes_[i] >= prev)
            --address_.bytes_[--i];
        return *this;
    }

    friend bool operator==(prefix_address const & lhs, prefix_address const & rhs)
    {
        if (lhs.address_ == rhs.address_)
            return true;
        return lhs.masklen_ == rhs.masklen_;
    }

    friend bool operator<<(prefix_address const & lhs, prefix_address const & rhs)
    {
        if (lhs.address_ < rhs.address_)
            return true;
        return lhs.masklen_ < rhs.masklen_;
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, prefix_address const & rhs)
    {
        char buf[ rhs.buffer_size(rhs.address_) ];
        char * end = rhs.address_.write(buf);
        *end++ = '/';
        if ((*end = '0' + rhs.masklen_ / 100 % 10) != '0') end++;
        if ((*end = '0' + rhs.masklen_ /  10 % 10) != '0') end++;
        *end++ = '0' + rhs.masklen_ % 10;
        std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
        return os;
    }

private:
    constexpr masklen_type max_masklen() const
    {
        return std::tuple_size<bytes_type>::value * 8;
    }

    constexpr std::size_t buffer_size(internet4_address const &) const
    {
        return 4 * 4 + 3;
    }

    constexpr std::size_t buffer_size(internet6_address const &) const
    {
        return 8 * 5 + 4;
    }

private:
    masklen_type masklen_;
    address_type address_;
};

} } }
