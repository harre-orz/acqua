#pragma once

#include <cstring>
#include <cwchar>
#include <iterator>
#include <string>
#include <type_traits>

#include <acqua/text/line_wrap_category.hpp>
#include <acqua/text/decode_exception.hpp>


namespace acqua { namespace text {

/*!
  BASE64変換テーブル
 */
template <typename CharT>
struct base64_transform_table
{
    static constexpr char const * s_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static constexpr std::size_t npos = 64;

    CharT operator[](int ch) const
    {
        return static_cast<CharT>(s_table[ch]);
    }

    std::size_t find(CharT ch) const
    {
        return std::find(s_table, s_table + npos, static_cast<char>(ch)) - s_table;
    }
};


/*!
  BASE64URL変換テーブル
 */
template <typename CharT>
struct base64_url_transform_table
{
    static constexpr char const * table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    static constexpr std::size_t npos = 64;

    CharT operator[](int ch) const
    {
        return static_cast<CharT>(table[ch]);
    }

    std::size_t find(CharT ch) const
    {
        return std::find(table, table + npos, static_cast<char>(ch)) - table;
    }
};

/*!
  BASE64変換
 */
template <template <typename CharT> class TransformTable>
struct basic_base64
{
    /*!
      BASE64テキストに符号化する
     */
    template <
        typename LineWrapCategory = no_line_wrap,
        typename InputIterator,
        typename OutputIterator
        >
    static void encode(InputIterator begin, InputIterator end, OutputIterator result)
    {
        LineWrapCategory line;
        TransformTable<typename std::iterator_traits<InputIterator>::value_type> table;
        int i = 0;
        char p_ch = 0;

        for(auto & it = begin; it != end; p_ch = *it++) {
            switch(i % 3) {
                case 0:
                    *result++ = ( table[ (*it & 0xfc) >> 2 ] );
                    break;
                case 1:
                    *result++ = ( table[ ((p_ch & 0x03) << 4) | ((*it & 0xf0) >> 4) ] );
                    break;
                case 2:
                    *result++ = ( table[ ((p_ch & 0x0f) << 2) | ((*it & 0xc0) >> 6) ] );
                    *result++ = ( table[ (*it & 0x3f) ] );
                    (line+=4)(result);
                    break;
            }

            ++i;
        }

        switch(i % 3) {
            case 1:
                *result++ = ( table[ ((p_ch & 0x03) << 4) ] );
                *result++ = ('=');
                *result++ = ('=');
                break;
            case 2:
                *result++ = ( table[ ((p_ch & 0x0f) << 2) ] );
                *result++ = ('=');
                break;
        }
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap,
        typename InputIterator
        >
    static OutputContainer encode(InputIterator begin, InputIterator end)
    {
        OutputContainer ret;
        encode<LineWrapCategory>(begin, end, std::back_inserter(ret));
        return ret;
    }

    template <
        typename OutputContainer,
        typename LineWrapCategory = no_line_wrap,
        typename InputContainer
        >
    static OutputContainer encode(InputContainer const & str)
    {
        return encode<OutputContainer, LineWrapCategory>(str.begin(), str.end());
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
        typename LineWrapCategory = no_line_wrap,
        typename InputIterator,
        typename Ch,
        typename Tr
        >
    static void encode(InputIterator begin, InputIterator end, std::basic_ostream<Ch, Tr> & os)
    {
        encode<LineWrapCategory>(begin, end, std::ostreambuf_iterator<Ch>(os));
    }

    /*!
      BASE64テキストを復元する
     */
    template <
        typename InputIterator,
        typename OutputIterator,
        bool SkipError = true
        >
    static bool decode(InputIterator begin, InputIterator end, OutputIterator result)
    {
        TransformTable<typename std::iterator_traits<InputIterator>::value_type> table;
        std::size_t i = 0;
        std::size_t st[5];

        for(auto & it = begin; it != end && *begin != '='; ++begin) {
            if ((st[4] = table.find(*it)) == table.npos) {
                if (SkipError)
                    continue;
                else
                    return false;
            }
            st[i++ % 4] = st[4];
            if (i % 4 == 0) {
                *result++ = ( ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4) );
                *result++ = ( ((st[1] & 0x0f) << 4) | ((st[2] & 0x3c) >> 2) );
                *result++ = ( ((st[2] & 0x03) << 6) | ((st[3] & 0x3f) >> 0) );
            }
        }

        switch(i % 4) {
            case 2:
                *result++ = ( ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4) );
                break;
            case 3:
                *result++ = ( ((st[0] & 0x3f) << 2) | ((st[1] & 0x30) >> 4) );
                *result++ = ( ((st[1] & 0x0f) << 4) | ((st[2] & 0x3c) >> 2) );
                break;
        }

        return true;
    }

    template <
        typename OutputContainer,
        typename InputIterator,
        bool SkipError = true
        >
    static OutputContainer decode(InputIterator begin, InputIterator end)
    {
        OutputContainer ret;
        if (!decode<InputIterator, decltype(std::back_inserter(ret)), SkipError>(begin, end, std::back_inserter(ret)))
            throw decode_exception();
        return ret;
    }

    template <
        typename OutputContainer,
        typename InputContainer,
        bool SkipError = true
        >
    static OutputContainer decode(InputContainer const & str)
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

    template <
        typename InputIterator,
        typename Ch,
        typename Tr,
        bool SkipError = true
        >
    static bool decode(InputIterator begin, InputIterator end, std::basic_ostream<Ch, Tr> & os)
    {
        return decode<InputIterator, decltype(std::ostreambuf_iterator<Ch>(os)), SkipError>(begin, end, std::ostreambuf_iterator<Ch>(os));
    }
};

/*!
  \class acqua::text::base64
 */
using base64 = basic_base64<base64_transform_table>;

/*!
  \class acqua::text::base64_url
 */
using base64_url = basic_base64<base64_url_transform_table>;

} }
