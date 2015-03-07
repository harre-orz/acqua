#pragma once

namespace acqua { namespace utility {

template <typename T>
struct is_char_traits
{
    static const bool value = false;
};

template <>
struct is_char_traits<char>
{
    static const bool value = true;
};

template <>
struct is_char_traits<wchar_t>
{
    static const bool value = true;
};

} }
