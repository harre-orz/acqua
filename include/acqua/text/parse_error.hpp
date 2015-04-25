#pragma once

#include <exception>
#include <boost/exception/exception.hpp>

namespace acqua { namespace text {

struct parse_error : virtual  std::exception, virtual boost::exception {};

} }
