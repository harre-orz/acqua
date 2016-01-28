/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/iostreams/newline_category.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/scope_exit.hpp>
#include <boost/mpl/bool.hpp>
#include <iostream>
#include <algorithm>

namespace acqua { namespace iostreams {

class base64_traits
{
public:
    static bool const padding = true;
    static int const npos = 64;

    char at(int ch) const
    {
        return tbl[std::min<uint>(static_cast<uint>(ch), static_cast<uint>(npos))];
    }

    int find(char ch) const
    {
        return static_cast<int>(std::find(tbl, tbl + npos, ch) - tbl);
    }

private:
    char const * const tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};


class base64_url_traits
{
public:
    static bool const padding = false;
    static int const npos = 64;

    char at(int ch) const
    {
        return tbl[std::min<uint>(static_cast<uint>(ch), static_cast<uint>(npos))];
    }

    int find(char ch) const
    {
        return static_cast<int>(std::find(tbl, tbl + npos, ch) - tbl);
    }

private:
    char const * const tbl = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
};


template <typename Traits>
class basic_base64_encoder
    : private Traits
    , public detail::newline_base< basic_base64_encoder<Traits> >
{
private:
    using base_type = typename basic_base64_encoder::base_type;
    using padding_type = typename boost::mpl::bool_<Traits::padding>::type;

public:
    struct category : boost::iostreams::output_filter_tag, boost::iostreams::closable_tag {};
    using char_type = char;
    using traits_type = Traits;

public:
    explicit basic_base64_encoder(newline nl = newline::none, std::size_t size = std::numeric_limits<std::size_t>::max())
        : base_type(nl, size) {}

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        BOOST_SCOPE_EXIT_ALL(this, ch) { prior_ = ch; ++i_; };
        switch(i_ % 3) {
            case 0:
                return base_type::put(sink, traits_type::at((ch & 0xFC) >> 2));
            case 1:
                return base_type::put_nobreak(sink, traits_type::at(((prior_ & 0x03) << 4) | ((ch & 0xF0) >> 4)));
            default: // 2
                return base_type::put_nobreak(sink, traits_type::at(((prior_ & 0x0F) << 2) | ((ch & 0xC0) >> 6)))
                    && base_type::put_nobreak(sink, traits_type::at((ch & 0x3F)));
        }
    }

    template <typename Sink>
    void close(Sink & sink)
    {
        BOOST_SCOPE_EXIT_ALL(this) { prior_ = 0; };
        line_break(sink);
    }

    // overrided by base_type
    template <typename Sink>
    bool line_break(Sink & sink)
    {
        BOOST_SCOPE_EXIT_ALL(this) { i_ = 0; };
        return line_break_impl(sink, padding_type())
            && base_type::line_break(sink);
    }

private:
    template <typename Sink>
    bool line_break_impl(Sink & sink, boost::mpl::true_)
    {
        switch(i_ % 3) {
            case 1:
                return boost::iostreams::put(sink, traits_type::at((prior_ & 0x03) << 4))
                    && boost::iostreams::put(sink, '=')
                    && boost::iostreams::put(sink, '=');
            case 2:
                return boost::iostreams::put(sink, traits_type::at((prior_ & 0x0F) << 2))
                    && boost::iostreams::put(sink, '=');
            default:
                return true;
        }
    }

    template <typename Sink>
    bool line_break_impl(Sink &, boost::mpl::false_) const
    {
        return true;
    }

private:
    std::size_t i_ = 0;
    char prior_ = {};
};

/*!
  base64 デコーダ.
  input_filter と output_filter の両方を指定できるが、１つのインスタンスに対してどちらか片方しか使用してはいけない
*/
template <typename Traits = base64_traits>
class basic_base64_decoder
    : private Traits
{
public:
    using char_type = char;
    using category = boost::iostreams::dual_use_filter_tag;
    using traits_type = Traits;

public:
    template <typename Source>
    int get(Source & src)
    {
        int ch;
        while((ch = boost::iostreams::get(src)) != EOF && ch != boost::iostreams::WOULD_BLOCK) {
            if (ch == '\r' || ch == '\n' || ch == '=' || (ch = traits_type::find(static_cast<char>(ch))) < 0 || traits_type::npos <= ch)
                continue;
            BOOST_SCOPE_EXIT_ALL(this, ch) { prior_ = ch; };
            switch(i_++ % 4) {
                case 1:
                    return ((prior_ & 0x3F) << 2 | (ch & 0x30) >> 4);
                case 2:
                    return ((prior_ & 0x0F) << 4 | (ch & 0x3C) >> 2);
                case 3:
                    return ((prior_ & 0x0F) << 6 | (ch & 0x3F) >> 0);
            }
        }
        return ch;
    }

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        if (ch == '\r' || ch == '\n' || ch == '=' || (ch = static_cast<char>(traits_type::find(ch))) < 0 || traits_type::npos <= ch)
            return true;
        BOOST_SCOPE_EXIT_ALL(this, ch) { prior_ = ch; };
        switch(i_++ % 4) {
            case 1:
                return boost::iostreams::put(sink, static_cast<char>((prior_ & 0x3F) << 2 | (ch & 0x30) >> 4));
            case 2:
                return boost::iostreams::put(sink, static_cast<char>((prior_ & 0x0F) << 4 | (ch & 0x3C) >> 2));
            case 3:
                return boost::iostreams::put(sink, static_cast<char>((prior_ & 0x0F) << 6 | (ch & 0x3F) >> 0));
        }
        return true;
    }

private:
    std::size_t i_ = 0;
    int prior_ = {};
};

using base64_encoder = basic_base64_encoder<base64_traits>;
using base64_decoder = basic_base64_decoder<base64_traits>;
using base64_url_encoder = basic_base64_encoder<base64_url_traits>;
using base64_url_decoder = basic_base64_decoder<base64_url_traits>;

} }
