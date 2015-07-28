/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>

namespace acqua { namespace json {

template <typename T>
class adapted;

template <typename CharT>
class adapted< std::basic_string<CharT> >
{
    using self_type = std::basic_string<CharT>;
    self_type & self_;

public:
    adapted(self_type & self)
        : self_(self) {}

    void data(self_type & val)
    {
        self_ = val;
    }
};

} }
