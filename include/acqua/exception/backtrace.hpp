/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <execinfo.h>
#include <stdlib.h>
}

#include <iostream>
#include <cstring>
#include <boost/units/detail/utility.hpp>

namespace acqua { namespace exception {

/*!
  バックトレースを出力する関数.

  コンパイル時に g++ -g -rdynamic を指定しないと、関数名がわからない
 */
inline std::ostream & backtrace(std::ostream & os)
{
    static const int MAX = 256;
    void * frames[MAX];
    int size = ::backtrace(frames, MAX);
    if (size <= 0 || size >= MAX)
        return os << "failed to backtrace (" << size << ")" << std::endl;
    char ** symbols = ::backtrace_symbols(frames, MAX);
    if (!symbols)
        return os << "failed to backtrace_symbols" << std::endl;

    try {
        for(int i = 0; i < size; ++i) {
            char * beg = symbols[i];
            char * end = beg + std::strlen(beg);
            char * a = std::find(beg, end, '(');
            char * b = std::find(a, end, '+');
            char * c = std::find(b, end, ')');
            if (c != end && std::distance(a, b) >= 1) {
                *const_cast<char *>(a++) = '\0';
                *const_cast<char *>(b++) = '\0';
                os << beg << ' ' << boost::units::detail::demangle(a) << ++c;
            } else {
                os << beg;
            }

            os << std::endl;
        }
    } catch(...) {
    }

    ::free(symbols);
    return os;
}

namespace detail {

//! errinfo_backtrace 用のタグ
class errinfo_backtrace_tag {};

}  // detail

} }
