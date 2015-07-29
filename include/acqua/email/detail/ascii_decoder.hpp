#pragma once

#include <algorithm>
#include <bitset>
#include <boost/locale.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace detail {

/*!
  7bit もしくは 8bit のテキストデコーダ.
 */
class ascii_decoder
{
public:
    explicit ascii_decoder(std::string const & charset, bool is_format_flowed, bool is_delete_space)
        : charset_(charset)
    {
        flags_[format_flowed] = is_format_flowed;
        flags_[delete_space] = is_delete_space;
    }

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        bool newline = true;
        if (flags_[format_flowed] && line.size() > 1 && std::isspace(*line.rbegin(), std::locale::classic()))
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
    static constexpr int format_flowed = 0;
    static constexpr int delete_space = 1;
    std::string charset_;
    std::bitset<2> flags_;
};

} } }
