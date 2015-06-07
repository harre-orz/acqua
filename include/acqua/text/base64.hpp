/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <algorithm>

namespace acqua { namespace text { namespace detail {

struct base64_base
{
    char tbl(char ch) const
    {
        return 0 <= ch && ch < npos ? tbl_[static_cast<int>(ch)] : 0;
    }

    char find(char ch) const
    {
        return std::find(tbl_, tbl_ + npos, ch) - tbl_;
    }

    char const * tbl_ = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const char npos = 64;
};

}

class base64 : detail::base64_base
{
public:
    static void decode(std::string const & in, std::string & out)
    {
        base64().do_decode(in.begin(), in.end(), std::back_inserter(out));
    }

private:
    template <typename It, typename Out>
    void do_decode(It it, It end, Out out)
    {
        char ch;
        for(; it != end; ++it) {
            switch(*it) {
                case '=':
                    prev_ = 0;
                    ++i_;
                    //break;
                case '\r': case '\n':
                    break;
                default:
                    if ((ch = find(*it)) == npos)
                        return;
                    switch(i_ % 4) {
                        case 1:
                            *out++ = (char)((prev_ & 0x3f) << 2 | (ch & 0x30) >> 4);
                            break;
                        case 2:
                            *out++ = (char)((prev_ & 0x0f) << 4 | (ch & 0x3c) >> 2);
                            break;
                        case 3:
                            *out++ = (char)((prev_ & 0x0f) << 6 | (ch & 0x3f) >> 0);
                            break;
                    }
                    prev_ = ch;
                    ++i_;
                    break;
            }
        }
    }

private:
    std::size_t i_ = 0;
    char prev_ = '\0';
};

} }
