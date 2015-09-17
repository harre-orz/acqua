#pragma once

#include <acqua/email/utils/base64_traits.hpp>

namespace acqua { namespace email { namespace utils {

template <typename CharT, typename Traits = base64_traits>
class basic_base64_encoder
    : private Traits
{
public:
    using char_type = CharT;
    using traits_type = Traits;

    explicit basic_base64_encoder()
    {}

    explicit basic_base64_encoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Source>
    void read(Source & src, std::string & line)
    {
        if (!src.good())
            return;

        std::basic_string<CharT> tmp;
        if (getline_eol(src, tmp)) {
            for(char ch : charset_.empty() ? acqua::string_cast<std::string>(tmp) : boost::locale::conv::from_utf(tmp, charset_)) {
                switch(i_++ % 3) {
                    case 0:
                        if (i_ > 1 && (i_ % 57) == 1)
                            line += "\r\n";
                        line += tbl[ (ch & 0xfc) >> 2 ];
                        break;
                    case 1:
                        line += tbl[ ((prior_ & 0x03) << 4) | ((ch & 0xf0) >> 4) ];
                        break;
                    case 2:
                        line += tbl[ ((prior_ & 0x0f) << 2) | ((ch & 0xc0) >> 6) ];
                        line += tbl[ (ch & 0x3f) ];
                        break;
                }
                prior_ = ch;
            }
        } else {
            switch(i_ % 3) {
                case 1:
                    line += tbl[ (prior_ & 0x03) << 4 ];
                    line += '=';
                    line += '=';
                    break;
                case 2:
                    line += tbl[ (prior_ & 0x0f) << 2];
                    line += '=';
                    break;
            }
            line += "\r\n\r\n";
        }
    }

    template <typename Source>
    void read_one(Source const & str, std::string & line) const
    {
        std::size_t i = 0;
        char prior;
        for(char ch : charset_.empty() ? acqua::string_cast<std::string>(str) : boost::locale::conv::from_utf(str, charset_)) {
            switch(i++ % 3) {
                case 0:
                    line += tbl[ (ch & 0xfc) >> 2 ];
                    break;
                case 1:
                    line += tbl[ ((prior & 0x03) << 4) | ((ch & 0xf0) >> 4) ];
                    break;
                case 2:
                    line += tbl[ ((prior & 0x0f) << 2) | ((ch & 0xc0) >> 6) ];
                    line += tbl[ (ch & 0x3f) ];
                    break;
            }
            prior = ch;
        }

        switch(i % 3) {
            case 1:
                line += tbl[ (prior & 0x03) << 4 ];
                line += '=';
                line += '=';
                break;
            case 2:
                line += tbl[ (prior & 0x0f) << 2];
                line += '=';
                break;
        }
    }

private:
    // 改行コードは消したくない
    template <typename Source, typename String>
    Source & getline_eol(Source & src, String & line)
    {
        line.clear();
        typename Source::char_type ch;
        while(src.get(ch)) {
            line += ch;
            if (ch == '\r' || ch == '\n') {
                if (ch == '\r' && src.peek() == '\n')
                    line += src.get();
                return src;
            }
        }

        return src;
    }

private:
    using traits_type::tbl;
    std::string charset_;
    std::size_t i_ = 0;
    char prior_;
};


template <typename CharT, typename Traits = base64_traits>
class basic_base64_raw_encoder
    : private Traits
{
public:
    using char_type = CharT;
    using traits_type = Traits;

    template <typename Source>
    void read(Source & src, std::string & line)
    {
        if (!src.good()) {
            if (prior_) {
                prior_ = 0;
                switch(i_ % 3) {
                    case 1:
                        line += tbl[ (prior_ & 0x03) << 4 ];
                        line += '=';
                        line += '=';
                        break;
                    case 2:
                        line += tbl[ (prior_ & 0x0f) << 2];
                        line += '=';
                        break;
                }
                line += "\r\n\r\n";
            }
            return;
        }

        CharT tmp[256];
        std::size_t size = src.read(tmp, sizeof(tmp)).gcount();
        for(char ch : acqua::string_cast<std::string>(tmp, tmp + size)) {
            switch(i_++ % 3) {
                case 0:
                    if (i_ > 1 && (i_ % 57) == 1)
                        line += "\r\n";
                    line += tbl[ (ch & 0xfc) >> 2 ];
                    break;
                case 1:
                    line += tbl[ ((prior_ & 0x03) << 4) | ((ch & 0xf0) >> 4) ];
                    break;
                case 2:
                    line += tbl[ ((prior_ & 0x0f) << 2) | ((ch & 0xc0) >> 6) ];
                    line += tbl[ (ch & 0x3f) ];
                    break;
            }
            prior_ = ch;
        }
    }

private:
    using traits_type::tbl;
    std::size_t i_ = 0;
    char prior_ = 0;
};

using base64_encoder = basic_base64_encoder<char>;
using base64_wencoder = basic_base64_encoder<wchar_t>;
using base64_raw_encoder = basic_base64_raw_encoder<char>;
using base64_raw_wencoder = basic_base64_raw_encoder<wchar_t>;

} } }
