#pragma once

#include <string>
#include <cstdlib>   // strtol
#include <iterator>
#include <cstring>
#include <boost/range/algorithm.hpp>

#include <acqua/text/line_wrap.hpp>
#include <acqua/text/decode_exception.hpp>

namespace acqua { namespace text {

struct quoted_printable
{
    template <typename LineWrap = no_line_wrap, typename InputIterator, typename OutputIterator>
    static void encode(InputIterator beg, InputIterator end, OutputIterator res, LineWrap & lw)
    {
        static_assert(sizeof(typename std::iterator_traits<InputIterator>::value_type) == 1, "");

        for(auto & it = beg; it != end; ++it) {
            lw(res);
            if (is_escape(*it)) {
                *res++ = '=';
                *res++ = do_enc((*it >> 4) & 0x0f);
                *res++ = do_enc((*it     ) & 0x0f);
                lw+=2;
            } else {
                *res++ = *it;
            }
            ++lw;
        }
    }

    template <typename LineWrap = no_line_wrap, typename InputIterator, typename OutputIterator>
    static void encode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        LineWrap lw;
        encode(beg, end, res, lw);
    }

    template <typename Output, typename LineWrap = no_line_wrap, typename InputIterator>
    static Output encode(InputIterator beg, InputIterator end)
    {
        LineWrap lw;
        Output res;
        encode(beg, end, std::back_inserter(res), lw);
        return res;
    }

    template <typename Output, typename LineWrap = no_line_wrap, typename Input>
    static Output encode(Input const & str)
    {
        LineWrap lw;
        Output res;
        encode(str.begin(), str.end(), std::back_inserter(res), lw);
        return res;
    }

    template <typename Output, typename LineWrap = no_line_wrap>
    static Output encode(char const * str)
    {
        LineWrap lw;
        Output res;
        encode(str, str + std::char_traits<char>::length(str), std::back_inserter(res), lw);
        return res;
    }

    template <bool SkipError = true, typename InputIterator, typename OutputIterator>
    static bool decode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        static_assert(sizeof(typename std::iterator_traits<InputIterator>::value_type) == 1, "");

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

    template <typename Output, bool SkipError = true, typename InputIterator>
    static Output decode(InputIterator beg, InputIterator end)
    {
        Output res;
        if (!decode<SkipError>(beg, end, std::back_inserter(res)))
            throw decode_exception();
        return res;
    }

    template <typename Output, bool SkipError = true, typename Input>
    static Output decode(Input const & str)
    {
        Output res;
        if (!decode<SkipError>(str.begin(), str.end(), std::back_inserter(res)))
            throw decode_exception();
        return res;
    }

    template <typename Output, bool SkipError = true>
    static Output decode(char const * str)
    {
        Output res;
        if (!decode<SkipError>(str, str + std::char_traits<char>::length(str), std::back_inserter(res)))
            throw decode_exception();
        return res;
    }

private:
    static bool is_escape(char ch)
    {
        return !(std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-');
    }

    static char do_enc(char hex)
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
