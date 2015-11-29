/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <arpa/inet.h>
}

#include <iostream>
#include <numeric>
#include <boost/system/error_code.hpp>
#include <boost/spirit/include/qi.hpp>

namespace acqua { namespace network { namespace detail {

struct address_impl_base
{
    template <typename T>
    static bool in_mask(T const & bytes) noexcept
    {
        for(auto it = bytes.begin(), end = bytes.end(); it != end; ++it) {
            switch(*it) {
                case 0b00000000:
                    if (it == bytes.begin()) return false;
                case 0b10000000:
                case 0b11000000:
                case 0b11100000:
                case 0b11110000:
                case 0b11111000:
                case 0b11111100:
                case 0b11111110:
                    return std::accumulate(++it, end, 0) == 0;
                case 0b11111111:
                    break;
                default:
                    return false;
            }
        }
        return true;
    }

    template <typename T>
    static void incr(T & bytes) noexcept
    {
        for(auto it = bytes.rbegin(); it != bytes.rend() && ++(*it++) == 0x00;);
    }

    template <typename T>
    static void decr(T & bytes) noexcept
    {
        for(auto it = bytes.rbegin(); it != bytes.rend() && --(*it++) == 0xFF;);
    }

    template <typename T>
    static void add(T & bytes, std::ptrdiff_t num) noexcept
    {
        if (num < 0)
            sub(bytes, -num);
        for(auto it = bytes.rbegin(); it != bytes.rend() && num; ++it) {
            auto rhs = static_cast<std::uint8_t>(num);
            *it += rhs;
            num >>= 8;
            num += (*it < rhs) ? 1 : 0;
        }
    }

    template <typename T>
    static void sub(T & bytes, std::ptrdiff_t num) noexcept
    {
        if (num < 0)
            add(bytes, -num);
        for(auto it = bytes.rbegin(); it != bytes.rend() && num; ++it) {
            auto lhs = *it;
            auto rhs = static_cast<std::uint8_t>(num);
            *it -= rhs;
            num >>= 8;
            num += (lhs < rhs) ? 1 : 0;
        }
    }

    template <typename T>
    static int netmask_length(T const & bytes) noexcept
    {
        int len = 0;
        for(auto ch : bytes) {
            if (ch == 0xFF) {
                len += 8;
            } else {
                for(int i = 7; i > 0 && (ch & (0x01 << i)); --i)
                    ++len;
            }
        }
        return len;
    }
};

template <typename T>
struct address_impl;

} } }
