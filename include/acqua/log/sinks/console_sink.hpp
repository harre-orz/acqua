#pragma once

#include <iostream>
#include <acqua/log/core.hpp>
#include <acqua/log/sinks/logger_sink.hpp>

namespace acqua { namespace log {

class core;

namespace sinks {

template <typename CharT, typename Mutex>
class console_sink
    : public logger_sink<CharT, Mutex>
{
    friend core;

private:
    console_sink(std::basic_ostream<CharT> & os)
        : os_(os) {}

    virtual void write(CharT const * str, std::size_t size) const override
    {
        os_.write(str, size);
        os_ << std::endl;
    }

private:
    std::basic_ostream<CharT> & os_;
};

} } }
