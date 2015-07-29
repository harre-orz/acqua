#pragma once

#include <algorithm>
#include <bitset>
#include <boost/locale.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace detail {

/*!
  7bit もしくは 8bit のテキストエンコーダ.
*/
class ascii_encoder
{
public:
    explicit ascii_encoder(std::string const & charset, bool is_format_flowed)
        : charset_(charset), is_format_flowed_(is_format_flowed) {}

    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        write_conv(sink, boost::locale::conv::from_utf<char>(line, charset_));
    }

private:
    template <typename Sink>
    void write_conv(Sink & sink, std::string const & line)
    {
        std::istreambuf_iterator<typename Sink::char_type> it(sink);
        if (is_format_flowed_) {
            std::size_t pos = 0;
            while((line.size() - pos) > 76) {
                std::copy_n(line.begin() + pos, 76, it);
                pos += 76;
                *it++ = ' ';
                *it++ = '\r';
                *it++ = '\n';
            }
            std::copy_n(line.begin() + pos, line.size() - pos, it);
        } else {
            std::copy_n(line.begin(), line.size(), it);
        }
        *it++ = '\r';
        *it++ = '\n';
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
    }

private:
    std::string charset_;
    bool is_format_flowed_;
};

} } }
