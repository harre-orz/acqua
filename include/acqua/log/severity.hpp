#pragma once

namespace acqua { namespace log {

//! ログレベル
enum severity_type {
    trace    = 0,
    debug    = 1,
    info     = 2,
    notice   = 3,
    warning  = 4,
    error    = 5,
    critical = 6,
    alert    = 7,
    emerg    = 8,
    disable  = 9,
};

inline std::ostream & operator<<(std::ostream & os, enum severity_type level)
{
    static char const * str[] = { "trace", "debug", "info", "notice", "warning", "error", "critical", "alert", "emerg", "" };
    return os << str[std::min<unsigned int>(level, disable)];
}

inline std::wostream & operator<<(std::wostream & os, enum severity_type level)
{
    static wchar_t const * str[] = { L"trace", L"debug", L"info", L"notice", L"warning", L"error", L"critical", L"alert", L"emerg", L"" };
    return os << str[std::min<unsigned int>(level, disable)];
}

inline char severity_symbol(enum severity_type level)
{
    char sym[] = { 'T', 'D', 'I', 'N', 'W', 'E', 'C', 'A', 'F', ' ' };
    return sym[std::min<unsigned int>(level, disable)];
}

} }
