#pragma once

#include <exception>
#include <boost/exception/exception.hpp>

namespace acqua { namespace text {

struct decode_exception : virtual std::exception, virtual boost::exception {};

} }
