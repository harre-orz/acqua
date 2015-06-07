/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <locale>

namespace acqua { namespace text {

class qprint
{
public:
    static void decode(std::string const & in, std::string & out)
    {
        qprint().do_decode(in.begin(), in.end(), std::back_inserter(out));
    }

private:
    template <typename It, typename Out>
    void do_decode(It it, It end, Out out)
    {
        for(; it != end; ++it) {
            if (*it == '=') {
                if (++it >= end) {
                    break;
                }
                char tmp[] = { 0,0,0 };
                if (!std::isxdigit(*it, std::locale::classic())) {
                    break;
                }
                tmp[0] = *it;
                if (++it >= end || !std::isxdigit(*it, std::locale::classic())) {
                    break;
                }
                tmp[1] = *it;
                *out++ = std::strtol(tmp, nullptr, 16);
            } else {
                *out++ = *it;
            }
        }
    }
};

} }
