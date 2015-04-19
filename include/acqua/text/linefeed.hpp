/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>

namespace acqua { namespace text {

enum linefeed_type {
    cr, ln, crln
};

inline std::ostream & operator<<(std::ostream & os, linefeed_type rhs)
{
    switch(rhs) {
        case linefeed_type::cr:
            os << '\r';
            break;
        case linefeed_type::ln:
            os << '\n';
            break;
        case linefeed_type::crln:
            os << '\r' << '\n';
            break;
    }

    return os;
}

} }
