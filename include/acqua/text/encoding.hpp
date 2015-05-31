#pragma once

#include <bitset>
#include <locale>
#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>
#include <boost/locale.hpp>
#include <acqua/string_cast.hpp>

namespace acqua { namespace text { namespace email_impl {

class no_decoding
{
public:
    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        sink << acqua::string_cast(line) << std::endl;
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }
};


class ascii_decoding
{
public:
    explicit ascii_decoding(std::string const & charset, bool is_format_flowed = false, bool is_delete_space = false)
        : charset_(charset)
    {
        flags_[format_flowed] = is_format_flowed;
        flags_[delete_space] = is_delete_space;
    }

    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        bool newline = true;

        if (flags_[format_flowed] && line.size() > 1 && std::isspace(*line.rbegin(), std::locale::classic()))
            // 行末がスペースのときに、改行しないようにするフラグ "Content-Type: format=flowed" で指定する
            // ただし、行末が "-- " というデータのときは例外として改行しなければならない
            newline = (line.size() >= 3 && (*line.rbegin()-1) == '-' && (*line.rbegin()-2) == '-');
        auto temp = boost::locale::conv::to_utf<typename Sink::char_type>(line, charset_);
        if (flags_[delete_space])
            boost::trim_right(temp);
        sink << acqua::string_cast(temp);
        if (newline)
            sink << std::endl;
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }

private:
    static const int format_flowed = 0;
    static const int delete_space = 1;
    std::string charset_;
    std::bitset<2> flags_;
};


class quoted_decoding
{
public:
    explicit quoted_decoding(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '=') {
                typename std::iterator_traits<decltype(it)>::value_type hex[] = { 0,0,0 };
                if (++it != line.end()) {
                    buffer_.push_back('=');
                    return;
                }
                hex[0] = *it;
                if (std::isxdigit(hex[0], std::locale::classic()) && ++it != line.end()) {
                    buffer_.push_back('=');
                    buffer_.push_back(hex[0]);
                    continue;
                }
                hex[1] = *it;
                if (std::isxdigit(hex[1], std::locale::classic())) {
                    buffer_.push_back('=');
                    buffer_.push_back(hex[0]);
                    buffer_.push_back(hex[1]);
                    continue;
                }
                buffer_.push_back(std::strtol(hex, nullptr, 16));
            } else {
                buffer_.push_back(*it);
            }
        }

        if (line.empty() || *line.rbegin() != '=') {
            sink << boost::locale::conv::to_utf<typename Sink::char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
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
    std::string charset_;
    std::string buffer_;
};


class base64_base
{
protected:
    char find(char ch) const
    {
        return (std::size_t)(std::find(tbl_, tbl_ + 64, ch) - tbl_);
    }

    char const * tbl_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};

class base64_file_decoding : base64_base
{
public:
    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
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


class base64_text_decoding : base64_base
{
public:
    explicit base64_text_decoding(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
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

} } }
