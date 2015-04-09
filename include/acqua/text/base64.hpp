#pragma once

#include <cstring>
#include <cwchar>
#include <iterator>
#include <string>
#include <type_traits>

#include <acqua/text/line_wrap.hpp>
#include <acqua/text/decode_exception.hpp>


namespace acqua { namespace text {

/*!
  BASE64変換テーブル
 */
struct base64_table
{
    static constexpr char const * s_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr std::size_t npos = 64;

    char operator[](int ch) const
    {
        return s_table[ch];
    }

    std::size_t find(char ch) const
    {
        return std::find(s_table, s_table + npos, ch) - s_table;
    }
};


/*!
  BASE64URL変換テーブル
 */
struct base64_url_table
{
    static constexpr char const * table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    static constexpr std::size_t npos = 64;

    char operator[](int ch) const
    {
        return table[ch];
    }

    std::size_t find(char ch) const
    {
        return std::find(table, table + npos, ch) - table;
    }
};

/*!
  BASE64変換
 */
template <typename Table = base64_table>
struct basic_base64
{
    /*!
      BASE64テキストに符号化する
     */
    template <typename LineWrap = no_line_wrap, typename InputIterator, typename OutputIterator>
    static void encode(InputIterator beg, InputIterator end, OutputIterator res, LineWrap & lw)
    {
        static_assert(sizeof(typename std::iterator_traits<InputIterator>::value_type) == 1, "");

        Table table;
        int i = 0;
        char p_ch = 0;

        for(auto & it = beg; it != end; p_ch = *it++) {
            switch(i % 3) {
                case 0:
                    lw(res); lw+=4;
                    *res++ = table[ (*it & 0xfc) >> 2 ];
                    break;
                case 1:
                    *res++ = table[ ((p_ch & 0x03) << 4) | ((*it & 0xf0) >> 4) ];
                    break;
                case 2:
                    *res++ = table[ ((p_ch & 0x0f) << 2) | ((*it & 0xc0) >> 6) ];
                    *res++ = table[ (*it & 0x3f) ];
                    break;
            }

            ++i;
        }

        switch(i % 3) {
            case 1:
                *res++ = ( table[ ((p_ch & 0x03) << 4) ] );
                *res++ = ('=');
                *res++ = ('=');
                break;
            case 2:
                *res++ = ( table[ ((p_ch & 0x0f) << 2) ] );
                *res++ = ('=');
                break;
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
        encode<LineWrap>(beg, end, std::back_inserter(res), lw);
        return res;
    }

    template <typename Output, typename LineWrap = no_line_wrap, typename Input>
    static Output encode(Input const & str)
    {
        LineWrap lw;
        Output res;
        encode<LineWrap>(str.begin(), str.end(), std::back_inserter(res), lw);
        return res;
    }

    template <typename Output, typename LineWrap = no_line_wrap>
    static Output encode(char const * str)
    {
        LineWrap lw;
        Output res;
        encode<LineWrap>(str, str + std::char_traits<char>::length(str), std::back_inserter(res), lw);
        return res;
    }

    template <typename LineWrap = no_line_wrap, typename InputIterator, typename Tr>
    static void encode(InputIterator begin, InputIterator end, std::basic_ostream<char, Tr> & os)
    {
        encode<LineWrap>(begin, end, std::ostreambuf_iterator<char>(os));
    }

    /*!
      BASE64テキストを復元する
     */
    template <bool SkipError = true, typename InputIterator, typename OutputIterator>
    static bool decode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        static_assert(sizeof(typename std::iterator_traits<InputIterator>::value_type) == 1, "");

        Table table;
        std::size_t i = 0;
        std::size_t st[5];

        for(auto & it = beg; it != end && *beg != '='; ++beg) {
            if ((st[4] = table.find(*it)) == table.npos) {
                if (SkipError)
                    continue;
                else
                    return false;
            }
            st[i++ % 4] = st[4];
            if (i % 4 == 0) {
                *res++ = ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4);
                *res++ = ((st[1] & 0x0f) << 4) | ((st[2] & 0x3c) >> 2);
                *res++ = ((st[2] & 0x03) << 6) | ((st[3] & 0x3f) >> 0);
            }
        }

        switch(i % 4) {
            case 2:
                *res++ = ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4);
                break;
            case 3:
                *res++ = ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4);
                *res++ = ((st[1] & 0x0f) << 4) | ((st[2] & 0x3c) >> 2);
                break;
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

    template <bool SkipError = true, typename InputIterator, typename Tr>
    static bool decode(InputIterator beg, InputIterator end, std::basic_ostream<char, Tr> & os)
    {
        return decode<SkipError>(beg, end, std::ostreambuf_iterator<char>(os));
    }
};

using base64 = basic_base64<base64_table>;

using base64_url = basic_base64<base64_url_table>;

} }
