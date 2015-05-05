#pragma once

#include <acqua/log/core.hpp>

namespace acqua { namespace log {

inline core::logging_line_logger detail_logging(koenig_lookup_tag, severity_type level, char const * func, char const * file, unsigned int line)
{
    return core::get_default().make_line_logger(level, func, file, line);
}

} }
