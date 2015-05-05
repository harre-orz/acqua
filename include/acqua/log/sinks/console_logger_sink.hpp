#pragma once

#include <iostream>
#include <acqua/log/core.hpp>
#include <acqua/log/sinks/logger_sink.hpp>

namespace acqua { namespace log { namespace sinks {

template <typename CharT, typename Mutex, typename Mode /* true_type is std::cout, false_type is std::cerr */ >
class basic_console_logger_sink
    : public logger_sink<CharT, Mutex>
{
public:
    using ostream_type = std::basic_ostream<CharT>;

    explicit basic_console_logger_sink()
        : os_( console(CharT(), Mode()) )
    {
    }

    virtual void write(CharT const * str, std::size_t size) const override
    {
        // TODO: スレッドセーフでない
        static Mutex s_mutex;

        std::lock_guard<Mutex> lock(s_mutex);
        os_.write(str, size);
        os_ << std::endl;
    }

private:
    static std::ostream & console(char, std::true_type) { return std::cout; }
    static std::ostream & console(char, std::false_type) { return std::cerr; }
    static std::wostream & console(wchar_t, std::true_type) { return std::wcout; }
    static std::wostream & console(wchar_t, std::false_type) { return std::wcerr; }

private:
    ostream_type & os_;
};

using stdout_logger_sink = basic_console_logger_sink<core::logger::char_type, core::logger::mutex_type, std::true_type>;
using stderr_logger_sink = basic_console_logger_sink<core::logger::char_type, core::logger::mutex_type, std::false_type>;

} } }
