#pragma once

namespace acqua { namespace email { namespace utils {

class base64_traits
{
protected:
    char find(char ch) const
    {
        return (std::size_t)(std::find(tbl, tbl + 64, ch) - tbl);
    }

    char const * tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
};

} } }
