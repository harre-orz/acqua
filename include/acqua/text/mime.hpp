#pragma once

namespace acqua { namespace text {

class rfc2047_tag {};
class rfc2231_tag {};

class mime
{
public:
    template <typename InputIterator, typename OutputIterator, typename LineWrap = no_line_wrap>
    static void encode(rfc2047_tag, InputIterator beg, InputIterator end, OutputIterator res, std::string const & charset = "UTF-8", LineWrap & lw)
    {
        if (in.empty())
            return;

        if (is_ascii(in)) {
            str_split(in, out, lw);
            return;
        }
    }

private:
    template <typename String>
    static bool is_ascii(String const & str)
    {
        for(auto const & ch : str)
            if (!std::isascii(ch))
                return false;
        return true;
    }

    template <typename In, typename Out, typename LineWrap>
    static void str_split(In const & in, Out & out, LineWrap & lw)
    {
        for(auto const & ch : in) {

        }
    }
};

} }
