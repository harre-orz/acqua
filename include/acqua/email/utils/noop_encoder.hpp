/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace utils {

/*!
  何もしないエンコーダ.
 */
template <typename CharT>
class basic_noop_encoder
{
public:
    using char_type = CharT;

    template <typename Source>
    void read(Source & src, std::string & line)
    {
        std::basic_string<CharT> tmp;
        if (!std::getline(src, tmp))
            return;

        line = acqua::string_cast<std::string>(tmp);
        line += "\r\n";
    }
};

using noop_encoder = basic_noop_encoder<char>;
using noop_wencoder = basic_noop_encoder<wchar_t>;

} } }
