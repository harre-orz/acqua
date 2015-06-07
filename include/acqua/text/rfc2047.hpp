#pragma once

#include <string>
#include <boost/locale.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace acqua { namespace text {

template <typename CharT>
class rfc2047_decoder
{
public:
    explicit rfc2047_decoder(std::string const & default_charset = "us-ascii")
        : charset_(default_charset) {}

    template <typename It, typename Sink>
    bool parse(It & it, It end, Sink & sink)
    {
        namespace xp = boost::xpressive;

        auto regex = xp::basic_regex<It>::compile("=\\?([[:alnum:]_=-]+)\\?([BbQq])\\?([[:alnum:]+_/=-]+)\\?=");
        xp::match_results<It> what;
        while(xp::regex_search(it, end, what, regex)) {
            switch(it[what.position(3)]) {
                case 'B': case 'b':
                    break;
                case 'Q': case 'q':
                    break;
            }
        }
    }

private:
    std::string charset_;
};

} }
