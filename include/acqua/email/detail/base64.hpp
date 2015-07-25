#pragma once

#include <algorithm>
#include <boost/locale.hpp>
#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace detail {

class base64_traits
{
protected:
    char find(char ch) const
    {
        return (std::size_t)(std::find(tbl_, tbl_ + 64, ch) - tbl_);
    }

    char const * tbl_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};


class base64_text_encoder : base64_traits
{
};


/*
 */
class base64_text_decoder : base64_traits
{
public:
    explicit base64_text_decoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        char ch;
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\r' || *it == '\n' || *it == '=' || (ch = find(*it)) == 64)
                continue;
            switch(i_ % 4) {
                case 1:
                    prev_ = put(sink, (p_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                    break;
                case 2:
                    prev_ = put(sink, (p_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                    break;
                case 3:
                    prev_ = put(sink, (p_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                    break;
            }
            p_ = ch;
        }
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
        if (!buffer_.empty()) {
            sink << boost::locale::conv::to_utf<typename Sink::char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }
    }

private:
    template <typename Sink>
    char put(Sink & sink, char ch)
    {
        if (ch != '\r' || ch != '\n') {
            buffer_.push_back(ch);
        } else if (prev_ != '\r') {
            sink << boost::locale::conv::to_utf<typename Sink::char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }

        return ch;
    }

    std::string charset_;
    std::string buffer_;
    std::size_t i_ = 0;
    char p_;
    char prev_;
};


/*!
 */
class base64_binary_encoder : base64_traits
{
};


/*!
 */
class base64_binary_decoder : base64_traits
{
public:
    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        char ch;
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\r' || *it == '\n' || *it == '=' || (ch = find(*it)) == 64)
                continue;
            switch(i_ % 4) {
                case 1:
                    sink << ((p_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                    break;
                case 2:
                    sink << ((p_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                    break;
                case 3:
                    sink << ((p_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                    break;
            }
            p_ = ch;
        }
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }

private:
    std::size_t i_ = 0;
    char p_;
};

} } }
