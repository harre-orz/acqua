#pragma once

#include <vector>
#include <memory>

namespace acqua { namespace log { namespace sinks {

template <typename CharT, typename Mutex>
class logger_sink
{
public:
    virtual void write(CharT const *, std::size_t) const = 0;
};

} } }
