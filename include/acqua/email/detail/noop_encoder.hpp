/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace detail {

/*!
  何もしないエンコーダ.
 */
class noop_encoder
{
public:
    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        sink << acqua::string_cast(line) << '\r' << '\n';
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
    }
};

} } }
