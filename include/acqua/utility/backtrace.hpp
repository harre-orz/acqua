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

#include <sstream>
#include <memory>
#include <algorithm>
#include <boost/units/detail/utility.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/detail/error_info_impl.hpp>

namespace acqua { namespace utility {

struct errinfo_backtrace_tag {};

/*!
  バックトレースを出力するマニピュレータ関数.

  コンパイル時に g++ -g -rdynamic を指定しないと、関数名がわからない
 */
inline std::ostream & backtrace(std::ostream & os)
{
    static const int MAX = 256;
    void * frames[MAX];
    int size = ::backtrace(frames, MAX);
    if (size <= 0 || size >= MAX)
        return os << "failed to backtrace (" << size << ")" << std::endl;
    std::unique_ptr<char*> symbols(::backtrace_symbols(frames, MAX));
    if (!symbols)
        return os << "failed to backtrace_symbols" << std::endl;

    for(int i = 0; i < size; ++i) {
        char * beg = symbols.get()[i];
        char * end = beg + std::char_traits<char>::length(beg);
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
    return os;
}

using errinfo_backtrace = boost::error_info<errinfo_backtrace_tag, void>;

} }

namespace boost {

template <>
class error_info<acqua::utility::errinfo_backtrace_tag, void>
    : public exception_detail::error_info_base
{
public:
    error_info()
    {
        std::ostringstream oss;
        oss << "---- backtrace ----" << std::endl
            << acqua::utility::backtrace
            << "-------------------" << std::endl;
        str_ = oss.str();
    }

private:
    std::string name_value_string() const
    {
        return str_;
    }

private:
    std::string str_;
};

}
