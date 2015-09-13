#pragma once

#include <bitset>

namespace acqua { namespace email { namespace utils {

class ascii_traits
{
public:
    explicit ascii_traits(bool is_format_flowed, bool is_delete_space)
    {
        flags_[format_flowed] = is_format_flowed;
        flags_[delete_space] = is_delete_space;
    }

    bool is_format_flowed() const
    {
        return flags_[format_flowed];
    }

    bool is_delete_space() const
    {
        return flags_[delete_space];
    }

private:
    static constexpr int format_flowed = 0;
    static constexpr int delete_space = 1;
    std::bitset<2> flags_;
};

} } }
