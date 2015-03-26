/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <sstream>

#include <boost/exception/error_info.hpp>
#include <boost/exception/detail/error_info_impl.hpp>
#include <boost/exception/to_string_stub.hpp>
#include <boost/exception_ptr.hpp>

#include <acqua/exception/backtrace.hpp>

namespace boost {

/*!
  error_info クラスで backtrace を出力するための特殊化クラス
 */
template <>
class error_info<acqua::exception::detail::errinfo_backtrace_tag, void>
    : public exception_detail::error_info_base
{
public:
    error_info()
    {
        std::ostringstream oss;
        oss << "---- backtrace ----" << std::endl
             << acqua::exception::backtrace
             << "-------------------" << std::endl;
        str_ = oss.str();
    }

private:
    std::string name_value_string() const
    {
        return str_;
    }

    std::string str_;
};

}

namespace acqua { namespace exception {

//! boost::exception オブジェクトに backtrace 情報を追加するクラス
typedef boost::error_info<detail::errinfo_backtrace_tag, void> errinfo_backtrace;

} }
