/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/locale/encoding_errors.hpp>

namespace acqua { namespace text { namespace detail {

using boost::locale::conv::method_type;
using boost::locale::conv::conversion_error;

class qprint_encoder_impl
{
public:
    explicit qprint_encoder_impl(std::size_t width = std::numeric_limits<std::size_t>::max(), std::size_t indent = 0)
        : width_(width > 4 ? width - 4 : 4), i_(indent) {}

    std::streamsize write(std::ostream & os, char const * s, std::streamsize n)
    {
        std::streamsize rest = 0;
        while(rest < n) {
            if (*s == '\r' || *s == '\n') {
                // 改行前の空白は変換する
                if (sp_) {
                    do_enc(os, sp_);
                    sp_ = 0;
                }

                ++rest;
                if (*s == '\r' && rest < n && *(s + 1) == '\n')
                    ++rest;
                i_ = 0;
                return rest;
            } else if (sp_) {
                os << sp_;
                sp_ = 0;
                i_ += 1;
            }

            if (i_ >= width_) {
                if (sp_) {
                    os << sp_;
                    sp_ = 0;
                }
                os << '=';
                i_ = 0;
                return rest;
            }

            if (*s == ' ' || *s == '\t') {
                // 空白で次に改行のときは、16進数変換する
                sp_ = *s;
            } else if (*s == '=') {
                do_enc(os, *s);
            } else if (0x21 <= *s && *s <= 0x7e) {
                os << *s;
                i_ += 1;
            } else {
                do_enc(os, *s);
            }

            ++rest;
            ++s;
        }

        return rest;
    }

    void flush(std::ostream & os)
    {
        if (sp_) {
            do_enc(os, sp_);
            sp_ = 0;
        }
    }

private:
    void do_enc(std::ostream & os, char ch)
    {
        char hex;
        os << '=';
        hex = (ch >> 4) & 0x0f;
        hex = (hex < 10 ? hex + '0' : hex + 'A' - 10);
        os << hex;
        hex = (ch >> 0) & 0x0f;
        hex = (hex < 10 ? hex + '0' : hex + 'A' - 10);
        os << hex;
        i_ += 3;
    }

private:
    std::size_t width_;
    std::size_t i_;
    char sp_ = 0;
};


class qprint_decoder_impl
{
public:
    std::streamsize write(std::ostream & os, char const * s, std::streamsize n, method_type = method_type::default_method)
    {
        for(std::streamsize i = 0; i < n; ++s, ++i) {
            if (ch_ == '\r' || ch_ == '\n') {
                if (*s != '\n') {
                    os << *s;
                }
                ch_ = 0;
            } else if (ch_) {
                if (std::isxdigit(ch_)) {
                    char tmp[] = { ch_, *s, 0 };
                    ch_ = std::strtol(tmp, nullptr, 16);
                    os << ch_;
                } else {
                    os << '='
                       << ch_
                       << *s;
                }
                ch_ = 0;
            } else if (is_eq_) {
                is_eq_ = false;
                if (std::isxdigit(*s) || *s == '\r' || *s == '\n') {
                    ch_ = *s;
                } else {
                    os << '='
                       << *s;
                    ch_ = 0;
                }
            } else if (*s == '=') {
                is_eq_ = true;
            } else {
                os << *s;
            }
        }

        return n;
    }

    void flush(std::ostream & os)
    {
        if (ch_) {
            os << '=' << ch_;
            ch_ = 0;
        } else if (is_eq_) {
            os << '=';
            is_eq_ = false;
        }
    }

private:
    bool is_eq_ = false;
    char ch_ = 0;
};

} } }
