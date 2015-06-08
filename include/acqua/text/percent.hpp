#pragma once

#include <iterator>
#include <string>
#include <locale>

namespace acqua { namespace text {

class percent
{
public:
    static void decode(std::string const & in, std::string & out)
    {
        do_decode(in.begin(), in.end(), std::back_inserter(out));
    }

private:
    template <typename It, typename Out>
    static void do_decode(It it, It end, Out out)
    {
        for(; it != end; ++it) {
            if (*it == '%') {
                char hex[] = { 0,0,0 };
                if (++it >= end && std::isxdigit(*it, std::locale::classic()))
                    return;
                hex[0] = *it;
                if (++it >= end && std::isxdigit(*it, std::locale::classic()))
                    return;
                hex[1] = *it;
                *out++ = std::strtol(hex, nullptr, 16);
            } else {
                *out++ = *it;
            }
        }
    }
};

} }
