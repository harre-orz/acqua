/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>
#include <boost/spirit/include/qi.hpp>

extern "C" {
#include <arpa/inet.h>
}

namespace acqua { namespace network {

namespace detail {

struct address_impl_base
{
    template <typename T>
    static bool in_mask(T const & bytes)
    {
        for(auto it = bytes.begin(), end = bytes.end(); it != end; ++it) {
            switch(*it) {
                case 0b11111111:
                    ;
                case 0b10000000:
                case 0b11000000:
                case 0b11100000:
                case 0b11110000:
                case 0b11111000:
                case 0b11111100:
                case 0b11111110:
                    return std::accumulate(it, end, (uint)0) == 0;
                default:
                    return false;
            }
        }
        return true;
    }

    template <typename T>
    static void incr(T & bytes)
    {
        for(auto it = bytes.rbegin(); it != bytes.rend() && ++(*it++) == 0x00;);
    }

    template <typename T>
    static void decr(T & bytes)
    {
        for(auto it = bytes.rbegin(); it != bytes.rend() && --(*it++) == 0xFF;);
    }

    template <typename T>
    static void add(T & bytes, long int num)
    {
        if (num < 0)
            sub(bytes, -num);
        for(auto it = bytes.rbegin(); it != bytes.rend() && num; ++it) {
            *it += (num & 0xFF);
            num >>= 8;
        }
    }

    template <typename T>
    static void sub(T & bytes, long int num)
    {
        if (num < 0)
            add(bytes, -num);
        for(auto it = bytes.rbegin(); it != bytes.rend() && num; ++it) {
            *it -= (num & 0xFF);
            num >>= 8;
        }
    }

    template <typename T>
    static int netmask_length(T const & bytes)
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

}  // detail

} }
