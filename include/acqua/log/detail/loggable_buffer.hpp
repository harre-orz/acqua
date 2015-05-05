#pragma once

#include <sstream>
#include <thread>
#include <boost/core/demangle.hpp>
#include <acqua/log/severity.hpp>
#include <acqua/log/detail/formatter.hpp>

namespace acqua { namespace log { namespace detail {

template <typename CharT>
struct loggable_buffer
    : std::basic_ostream<CharT>
    , formatter< loggable_buffer<CharT>, boost::posix_time::ptime>
{
    explicit loggable_buffer(severity_type level, char const * func, char const * file, unsigned int line, std::type_info const & type)
        : std::basic_ostream<CharT>(&buffer)
        , level(level)
        , func(func)
        , file(file)
        , line(line)
        , type(type)
        , tid(std::this_thread::get_id()) {}

    std::basic_stringbuf<CharT> buffer;
    severity_type level;
    char const * func;
    char const * file;
    unsigned int line;
    std::type_info const & type;
    std::thread::id tid;

    template <typename Out>
    void replace_logger_info(Out & os) const
    {
        boost::core::scoped_demangled_name name(type.name());
        std::copy_n(name.get(), std::strlen(name.get()), std::ostreambuf_iterator<typename Out::char_type>(os));
    }
};

} } }
