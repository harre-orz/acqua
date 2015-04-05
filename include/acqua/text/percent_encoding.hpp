#pragma once

#include <string>
#include <acqua/utility/is_charactor.hpp>
#include <acqua/text/decode_exception.hpp>

namespace acqua { namespace text {

class percent_encoding
{
public:
    template <typename InputIterator, typename OutputIterator>
    static void encode(InputIterator beg, InputIterator end, OutputIterator res)
    {
        for(auto it = beg; it != end; ++it) {
            if (is_escape(*it)) {
                *res++ = '=';
                *res++ = do_enc((*it >> 4) & 0x0f);
                *res++ = do_enc((*it     ) & 0x0f);
            } else {
                *res++ = *it;
            }
        }
    }

    template <typename OutputContainer, typename InputIterator>
    static OutputContainer encode(InputIterator beg, InputIterator end)
    {
        OutputContainer res;
        encode<InputIterator, decltype(std::back_inserter(res))>(beg, end, std::back_inserter(res));
        return res;
    }

    template <typename OutputContainer, typename InputContainer>
    static OutputContainer encode(InputContainer const & str)
    {
        return encode<OutputContainer, decltype(str.begin())>(str.begin(), str.end());
    }

    template <typename OutputContainer, typename Ch, typename std::enable_if<acqua::utility::is_charactor<Ch>::value_type>::type * = nullptr>
    static OutputContainer encode(Ch const * str)
    {
        return encode<OutputContainer, Ch const *>(str, str + std::char_traits<Ch>::length(str));
    }

    template <typename String>
    static bool decode(String & str)
    {
        typename String::value_type tmp[] = {0,0,0};
        for(auto pos = str.size() - 1; pos != String::npos && (pos = str.rfind('%', pos)) != String::npos; --pos) {
            if (pos + 2 < str.size() && std::isxdigit(str[pos+1]) && std::isxdigit(str[pos+2])) {
                tmp[0] = str[pos+1];
                tmp[1] = str[pos+2];
                str.replace(pos, 3, 1, std::stoi(tmp, nullptr, 16));
            }
        }

        return true;
    }
};

} }
