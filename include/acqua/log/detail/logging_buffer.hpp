#pragma once

#include <sstream>
#include <thread>
#include <acqua/log/severity.hpp>
#include <acqua/log/detail/formatter.hpp>

namespace acqua { namespace log { namespace detail {

template <typename CharT>
struct logging_buffer
    : std::basic_ostream<CharT>
    , formatter< logging_buffer<CharT>, boost::posix_time::ptime>
{
    explicit logging_buffer(severity_type level, char const * func, char const * file, unsigned int line)
        : std::basic_ostream<CharT>(&buffer)
        , level(level), func(func), file(file), line(line)
        , tid(std::this_thread::get_id()) {}

    std::basic_stringbuf<CharT> buffer;
    severity_type level;
    char const * func;
    char const * file;
    unsigned int line;
    std::thread::id tid;
};

} } }
