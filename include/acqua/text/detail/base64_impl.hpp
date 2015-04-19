/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <algorithm>
#include <boost/locale/encoding_errors.hpp>

namespace acqua { namespace text { namespace detail {

using boost::locale::conv::method_type;
using boost::locale::conv::conversion_error;

struct base64_base
{
    char tbl(char ch) const
    {
        return 0 <= ch && ch < npos ? tbl_[static_cast<int>(ch)] : 0;
    }

    char find(char ch) const
    {
        return std::find(tbl_, tbl_ + npos, ch) - tbl_;
    }

    char const * tbl_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char npos = 64;
};


class base64_encoder_impl : base64_base
{
public:
    explicit base64_encoder_impl(std::size_t width = std::numeric_limits<std::size_t>::max(), std::size_t indent = 0)
        : width_(width > 4 ? width - 4 : 4), i_(indent + (4 - indent % 4) % 4) {}

    std::streamsize write(std::ostream & os, char const * s, std::streamsize n)
    {
        std::streamsize rest = 0;
        while(rest < n) {
            switch(i_ % 4) {
                case 0:
                    if (i_ >= width_) {
                        i_ = 0;
                        return rest;
                    }

                    os << tbl((*s & 0xfc) >> 2);
                    i_ += 1;
                    break;
                case 1:
                    os << tbl((ch_ & 0x03) << 4 | ((*s & 0xf0) >> 4));
                    i_ += 1;
                    break;
                case 2:
                    os << tbl((ch_ & 0x0f) << 2 | ((*s & 0xc0) >> 6));
                    os << tbl(*s & 0x3f);
                    i_ += 2;
                    break;
            }
            ch_ = *s++;
            ++rest;
        }

        return rest;
    }

    void flush(std::ostream & os)
    {
        switch(i_ % 4) {
            case 0:
                break;
            case 1:
                os << tbl((ch_ & 0x03) << 4);
                os << '=';
                os << '=';
                i_ += 3;
                break;
            case 2:
                os << tbl((ch_ & 0x0f) << 2);
                os << '=';
                i_ += 2;
                break;
        }
    }

private:
    std::size_t width_;
    std::size_t i_ = 0;
    char ch_ = 0;
};


class base64_decoder_impl : base64_base
{
public:
    std::streamsize write(std::ostream & os, char const * s, std::streamsize n, method_type how = method_type::default_method)
    {
        char ch;
        for(std::streamsize i = 0; i < n; ++s, ++i) {
            if (*s == '\r' || *s == '\n' || *s == '=')
                continue;
            if ((ch = find(*s)) == npos) {
                if (how == method_type::stop)
                    throw conversion_error();
                continue;
            }

            switch(i_ % 4) {
                case 1:
                    os << (char)((ch_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                    break;
                case 2:
                    os << (char)((ch_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                    break;
                case 3:
                    os << (char)((ch_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                    break;
            }
            ch_ = ch;
            i_ += 1;
        }

        return n;
    }

    void flush(std::ostream &)
    {
        i_ = 0;
    }

private:
    std::size_t i_ = 0;
    char ch_ = 0;
};


} } }
