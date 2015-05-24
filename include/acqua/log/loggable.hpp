#pragma once

#include <acqua/log/core.hpp>

namespace acqua { namespace log {

template <typename Derived, typename Tag = typename core::default_tag>
class loggable
{
protected:
    static core::loggable_line_logger detail_logging(koenig_lookup_tag, severity_type level, char const * func, char const * file, unsigned int line)
    {
        return core::get<Tag>()->make_line_logger(level, func, file, line, typeid(Derived));
    }
};

} }
