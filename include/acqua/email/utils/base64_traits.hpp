#pragma once

namespace acqua { namespace email { namespace utils {

class base64_traits
{
protected:
    static char const npos = 64;

    char const * tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    char find(char ch) const
    {
        return std::find(tbl, tbl + npos, ch) - tbl;
    }
};

} } }
