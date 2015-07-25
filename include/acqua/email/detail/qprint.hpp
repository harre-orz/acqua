#pragma once

#include <iterator>
#include <boost/locale.hpp>

namespace acqua { namespace email { namespace detail {

/*!
  quoted-printable 形式のテキストにエンコード.
 */
class qprint_encoder
{
public:
    explicit qprint_encoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        write_encode(sink, boost::locale::conv::from_utf<char>(line, charset_));
    }

private:
    template <typename Sink>
    void write_encode(Sink & sink, std::string const & line)
    {
        std::size_t i = 0;
        for(char ch : line) {
            if (i >= 76) {
                i = 0;
                sink << '=' << '\r' << '\n';
            }
            if (std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-') {
                sink << ch;
                ++i;
            } else {
                do_escape(sink, ch);
                i += 3;
            }
        }
        sink << '\r' << '\n';
    }

    template <typename Sink>
    void do_escape(Sink & sink, char ch)
    {
        char hex;
        sink << '=';
        hex = (ch & 0xf0) >> 4;
        sink << ((hex < 10 ? '0' : 'A' - 10) + hex);
        hex = (ch & 0x0f);
        sink << ((hex < 10 ? '0' : 'A' - 10) + hex);
    }

private:
    std::string charset_;
};

/*!
  quoted-printable 形式のテキストからデコード.
 */
class qprint_decoder
{
public:
    explicit qprint_decoder(std::string const & charset)
        : charset_(charset) {}

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        for(auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '=') {
                char hex[] = {0,0,0};
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
    };

private:
    std::string charset_;
    std::string buffer_;
};

} } }
