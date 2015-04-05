#pragma once

#include <string>
#include <cstdlib>   // strtol
#include <iterator>
#include <cstring>
#include <boost/range/algorithm.hpp>

#include <acqua/text/line_wrap_category.hpp>
#include <acqua/text/decode_exception.hpp>

namespace acqua { namespace text {

struct quoted_printable
{
    template <
        typename LineWrapCategory = no_line_wrap,
        typename InputIterator,
        typename OutputIterator
        >
    static void encode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        LineWrapCategory line;

        for(auto & it = beg; it != end; ++it) {
            ++line;
            if (is_escape(*it)) {
                *res++ = '=';
                *res++ = do_enc((*it >> 4) & 0x0f);
                *res++ = do_enc((*it     ) & 0x0f);
                (line+=2)(res);
            } else
                *res++ = *it;
        }
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap,
        typename InputIterator
        >
    static OutputContainer encode(InputIterator beg, InputIterator end)
    {
        OutputContainer res;
        encode<LineWrapCategory>(beg, end, std::back_inserter(res));
        return res;
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap,
        typename InputContainer
        >
    static OutputContainer encode(const InputContainer & src)
    {
        return encode<OutputContainer, LineWrapCategory>(src.begin(), src.end());
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap
        >
    static OutputContainer encode(char const * str)
    {
        return encode<OutputContainer, LineWrapCategory>(str, str + std::strlen(str));
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap
        >
    static OutputContainer encode(wchar_t const * str)
    {
        return encode<OutputContainer, LineWrapCategory>(str, str + std::wcslen(str));
    }

    template <
        typename InputIterator,
        typename OutputIterator,
        bool SkipError = true
        >
    static bool decode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        for(auto & it = beg; it != end; ++it) {
            if (*it == '=') {
                if (++it == end) {
                    return SkipError;
                } else if (*it == '\r' || *it == '\n' ) {
                    if (*it == '\r') {
                        if (++it == end) return true;
                    }
                    if (*it == '\n') ++it;
                } else {
                    char hex1 = (char)*it;
                    if (++it == end) return SkipError;
                    char hex2 = (char)*it;
                    if (!(do_dec(hex1, hex2, res) || SkipError))
                        return false;
                }
            } else
                *res++ = *it;
        }

        return true;
    }

    template <
        typename OutputContainer,
        typename InputIterator,
        bool SkipError = true
        >
    static OutputContainer decode(InputIterator beg, InputIterator end)
    {
        OutputContainer res;
        if (!decode<InputIterator, decltype(std::back_inserter(res)), SkipError>(beg, end, std::back_inserter(res)))
            throw decode_exception();
        return res;
    }

    template <
        typename OutputContainer,
        typename InputContainer,
        bool SkipError = true
        >
    static OutputContainer decode(const InputContainer & str)
    {
        return decode<OutputContainer, decltype(str.begin()), SkipError>(str.begin(), str.end());
    }

    template <
        typename OutputContainer,
        bool SkipError = true
        >
    static OutputContainer decode(char const * str)
    {
        return decode<OutputContainer, decltype(str), SkipError>(str, str + std::strlen(str));
    }


    template <
        typename OutputContainer,
        bool SkipError = true
        >
    static OutputContainer decode(wchar_t const * str)
    {
        return decode<OutputContainer, decltype(str), SkipError>(str, str + std::wcslen(str));
    }

private:
    template <typename CharT>
    static bool is_escape(CharT ch)
    {
        return !(std::isalnum(static_cast<char>(ch)) || ch == '.' || ch == '_' || ch == '-');
    }

    template <typename CharT>
    static char do_enc(CharT hex)
    {
        return (hex < 10 ? '0' + hex : 'A' - 10 + hex);
    }

    template <typename OutputIterator>
    static bool do_dec(char hex1, char hex2, OutputIterator & out)
    {
        char data[] = { hex1, hex2, '\0' };
        char * endptr;

        int digit = std::strtol(data, &endptr, 16);
        if (endptr != data+2) return false;
        *out = digit;
        return true;
    }
};

} }
