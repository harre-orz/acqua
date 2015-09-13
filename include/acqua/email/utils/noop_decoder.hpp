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
  何もしないデコーダ.
 */
template <typename CharT>
class basic_noop_decoder
{
public:
    using char_type = CharT;

    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        sink << acqua::string_cast(line) << std::endl;
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }
};

using noop_decoder = basic_noop_decoder<char>;

} } }
