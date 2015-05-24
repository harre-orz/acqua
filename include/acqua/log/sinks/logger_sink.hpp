#pragma once

#include <vector>
#include <memory>

namespace acqua { namespace log {

class core;

namespace sinks {

template <typename CharT, typename Mutex>
class logger_sink
{
public:
    virtual ~logger_sink() {}
    virtual void write(CharT const *, std::size_t) const = 0;

protected:
    Mutex mutex_;
};

} } }
