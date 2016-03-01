#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/newline_category.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/scope_exit.hpp>
#include <iostream>
#include <algorithm>

namespace acqua { namespace iostreams {

struct base64_traits
{
    static bool const padding = true;
    static int const npos = 64;

    char enc(int ch) const
    {
        return tbl[std::min<uint>(static_cast<uint>(ch), static_cast<uint>(npos))];
    }

    int dec(char ch) const
    {
        return static_cast<int>(std::find(tbl, tbl + npos, ch) - tbl);
    }

private:
    char const * const tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};


struct base64_url_traits
{
    static bool const padding = false;
    static int const npos = 64;

    char enc(int ch) const
    {
        return tbl[std::min<uint>(static_cast<uint>(ch), static_cast<uint>(npos))];
    }

    int dec(char ch) const
    {
        return static_cast<int>(std::find(tbl, tbl + npos, ch) - tbl);
    }

private:
    char const * const tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
};


template <typename Traits>
class basic_base64_encoder
    : private Traits
{
private:
    using Traits::enc;
    using Traits::padding;

public:
    struct category : boost::iostreams::dual_use_filter_tag, boost::iostreams::closable_tag {};
    using char_type = char;
    using traits_type = Traits;

public:
    explicit basic_base64_encoder(newline nl = newline::none, std::size_t size = std::numeric_limits<std::size_t>::max())
        : nl_(nl)
    {
        if (size < 4) {
            size = 4;
        } else {
            // 改行コードを含んで、size を超えないように調整する
            switch(nl_) {
                case newline::none:
                    size = std::numeric_limits<std::size_t>::max();
                    break;
                case newline::crln:
                    --size;
                    //break;
                case newline::cr: case newline::ln:
                    --size;
                    break;
            }
            max_ = size - (size % 4);  // max_ は必ず4の倍数にする
        }
    }

    template <typename Source>
    int get(Source & src)
    {
        int res;
        int ch;

        switch(cnt_ % 4) {
            case 0:
                if (cnt_ >= max_)
                    if ((ch = get_break()) != EOF)
                        return ch;

                if (prior_ == EOF) return EOF;
                ch = boost::iostreams::get(src);
                if (ch == EOF) {
                    prior_ = EOF;
                    if (cnt_ == 0) {
                        return EOF;
                    } else {
                        max_ = cnt_;  // 最後に改行を入れる調整
                        return get_break();
                    }
                }
                res = enc((ch & 0xFC) >> 2);
                prior_ = ch;
                break;
            case 1:
                if (cnt_ >= max_)
                    return get_break();
                ch = boost::iostreams::get(src);
                res = (ch == EOF) ? enc(((prior_ & 0x03)) << 4)
                    : enc(((prior_ & 0x03) << 4) | ((ch & 0xF0) >> 4));
                prior_ = ch;
                break;
            case 2:
                if (prior_ == EOF) {
                    if (padding) {
                        res = '=';
                    } else {
                        max_ = cnt_;  // 最後に改行を入れる調整
                        return get_break();
                    }
                } else {
                    ch = boost::iostreams::get(src);
                    res = (ch == EOF) ? enc((prior_ & 0x0F) << 2)
                        : enc(((prior_ & 0x0F) << 2) | ((ch & 0xC0) >> 6));
                    prior_ = ch;
                }
                break;
            case 3:
                if (prior_ == EOF) {
                    if (padding) {
                        res = '=';
                        if (cnt_ > 0) max_ = cnt_ + 1;  // 最後に改行を入れる調整
                    } else {
                        max_ = cnt_;  // 最後に改行を入れる調整
                        return get_break();
                    }
                } else {
                    res = enc(prior_ & 0x3F);
                }
                break;
        }

        ++cnt_;
        return res;
    }

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        bool res;

        switch(cnt_ % 4) {
            case 0:
                if (cnt_ >= max_)
                    if (!put_break(sink))
                        return false;

                res = boost::iostreams::put(sink, enc((ch & 0xFC) >> 2));
                break;
            case 1:
                res = boost::iostreams::put(sink, enc(((prior_ & 0x03) << 4) | ((ch & 0xF0) >> 4)));
                break;
            case 2:
                res =  boost::iostreams::put(sink, enc(((prior_ & 0x0F) << 2) | ((ch & 0xC0) >> 6)))
                    && boost::iostreams::put(sink, enc(ch & 0x3F));
                ++cnt_;
                break;
        }

        ++cnt_;
        prior_ = ch;
        return res;
    }

    template <typename Device>
    void close(Device & dev, std::ios_base::openmode which)
    {
        if (which == std::ios_base::out && cnt_ > 0) {
            switch(cnt_ % 4) {
                case 1:
                    boost::iostreams::put(dev, enc((prior_ & 0x03) << 4));
                    if (padding) {
                        boost::iostreams::put(dev, '=');
                        boost::iostreams::put(dev, '=');
                    }
                    break;
                case 2:
                    boost::iostreams::put(dev, enc((prior_ & 0x0F) << 2));
                    if (padding) {
                        boost::iostreams::put(dev, '=');
                    }
                    break;
            }

            // 最後に改行を入れる
            put_break(dev);
        }
    }

private:
    template <typename Sink>
    bool put_break(Sink & sink)
    {
        cnt_ = 0;
        switch(nl_) {
            case newline::none:
                return true;
            case newline::ln:
                return boost::iostreams::put(sink, '\n');
            case newline::cr:
                return boost::iostreams::put(sink, '\r');
            case newline::crln:
                return boost::iostreams::put(sink, '\r')
                    && boost::iostreams::put(sink, '\n');
        }
    }

    int get_break()
    {
        switch(nl_) {
            case newline::ln:
                cnt_ = 0;
                return '\n';
            case newline::cr:
                cnt_ = 0;
                return  '\r';
            case newline::crln:
                if (cnt_ == max_) {
                    ++cnt_;  // 改行を入れる調整
                    return '\r';
                } else {
                    cnt_ = 0;
                    return '\n';
                }
            default:
                cnt_ = 0;
                return EOF;
        }
    }

private:
    newline nl_;
    std::size_t max_;
    std::size_t cnt_ = 0;
    int prior_ = {};
};


/*!
  base64 デコーダ.
*/
template <typename Traits = base64_traits>
class basic_base64_decoder
    : private Traits
{
private:
    using Traits::dec;
    using Traits::npos;

public:
    using char_type = char;
    using category = boost::iostreams::dual_use_filter_tag;

public:
    template <typename Source>
    int get(Source & src)
    {
        int ch;
        while((ch = boost::iostreams::get(src)) != EOF && ch != boost::iostreams::WOULD_BLOCK) {
            if (ch == '\r' || ch == '\n' || ch == '=' || (ch = dec(static_cast<char>(ch))) < 0 || npos <= ch)
                continue;
            BOOST_SCOPE_EXIT_ALL(this, ch) { prior_ = ch; };
            switch(cnt_++ % 4) {
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
        if (ch == '\r' || ch == '\n' || ch == '=' || (ch = static_cast<char>(dec(ch))) < 0 || npos <= ch)
            return true;
        BOOST_SCOPE_EXIT_ALL(this, ch) { prior_ = ch; };
        switch(cnt_++ % 4) {
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
    std::size_t cnt_ = 0;
    int prior_ = {};
};

using base64_encoder = basic_base64_encoder<base64_traits>;
using base64_decoder = basic_base64_decoder<base64_traits>;
using base64_url_encoder = basic_base64_encoder<base64_url_traits>;
using base64_url_decoder = basic_base64_decoder<base64_url_traits>;

} }
