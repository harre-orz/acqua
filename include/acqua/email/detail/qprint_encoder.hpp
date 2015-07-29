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


} } }
