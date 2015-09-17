#pragma once

#include <acqua/email/utils/base64_traits.hpp>

namespace acqua { namespace email { namespace utils {

template <typename CharT, typename Traits = base64_traits>
class basic_base64_decoder
    : private Traits
{
public:
    using char_type = CharT;
    using traits_type = Traits;

    explicit basic_base64_decoder()
    {}

    explicit basic_base64_decoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        char ch;
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\r' || *it == '\n' || *it == '=' || (ch = traits_type::find(*it)) == traits_type::npos)
                continue;
            switch(i_++ % 4) {
                case 1:
                    last_ = put(sink, (prior_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                    break;
                case 2:
                    last_ = put(sink, (prior_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                    break;
                case 3:
                    last_ = put(sink, (prior_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                    break;
            }
            prior_ = ch;
        }
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
        if (!buffer_.empty()) {
            if (charset_.empty())
                sink << acqua::string_cast(buffer_);
            else
                sink << boost::locale::conv::to_utf<char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }
    }

private:
    template <typename Sink>
    char put(Sink & sink, char ch)
    {
        if (ch != '\r' && ch != '\n') {
            buffer_.push_back(ch);
        } else if (last_ != '\r') {
            if (charset_.empty())
                sink << acqua::string_cast(buffer_);
            else
                sink << boost::locale::conv::to_utf<char_type>(buffer_, charset_) << std::endl;
            buffer_.clear();
        }
        return ch;
    }

    std::string charset_;
    std::string buffer_;
    std::size_t i_ = 0;
    char prior_;
    char last_;
};


template <typename CharT, typename Traits = base64_traits>
class basic_base64_raw_decoder
    : private Traits
{
public:
    using char_type = CharT;
    using traits_type = Traits;

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        std::cout << line << std::endl;
        char ch;
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\r' || *it == '\n' || *it == '=' || (ch = traits_type::find(*it)) == 64)
                continue;
            switch(i_++ % 4) {
                case 1:
                    sink.put((prior_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                    break;
                case 2:
                    sink.put((prior_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                    break;
                case 3:
                    sink.put((prior_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                    break;
            }
            prior_ = ch;
        }
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }

private:
    std::size_t i_ = 0;
    char prior_;
};

using base64_decoder = basic_base64_decoder<char>;
using base64_raw_decoder = basic_base64_raw_decoder<char>;

} } }
