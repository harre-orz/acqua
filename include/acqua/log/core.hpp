#pragma once

#include <mutex>
#include <memory>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <boost/serialization/singleton.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <acqua/log/severity.hpp>
#include <acqua/log/detail/line_logger.hpp>
#include <acqua/log/detail/logging_buffer.hpp>
#include <acqua/log/detail/loggable_buffer.hpp>
#include <acqua/log/sinks/logger_sink.hpp>

namespace acqua { namespace log {

//! ADL(Argument Dependent name Loopup)を発生させるためのタグ
class koenig_lookup_tag {};


class core
    : private boost::serialization::singleton<core>
{
    friend boost::serialization::detail::singleton_wrapper<core>;
    using singleton = boost::serialization::singleton<core>;
    using char_type = char;
    using mutex_type = std::mutex;

    core()
    {
        find_or_construct(std::type_index(typeid(default_tag)));
    }

public:
    //! デフォルトのロガータグ
    struct default_tag {};

    class logger;

    using logging_line_logger = detail::line_logger<logger, detail::logging_buffer<char_type> >;
    using loggable_line_logger = detail::line_logger<logger,  detail::loggable_buffer<char_type> >;

    //! ロガー
    class logger
    {
    public:
        using char_type = core::char_type;
        using mutex_type = core::mutex_type;
        using string_type = std::basic_string<char_type>;

        logger(core & core_)
            : name_(core_.name_) {}

        std::string const & logger_name() const
        {
            return name_;
        }

        void logger_name(std::string const & name)
        {
            name_ = name;
        }

        template <typename Sink>
        void push(Sink const & sink)
        {
            // TODO: スレッドセーフでない
            sinks_.emplace_back(new Sink(sink));
        }

        void pop()
        {
            // TODO: スレッドセーフでない
            sinks_.pop_back();
        }

        logging_line_logger make_line_logger(severity_type level, char const * func, char const * file, unsigned int line)
        {
            return logging_line_logger(*this, new detail::logging_buffer<char_type>(level, func, file, line));
        }

        loggable_line_logger make_line_logger(severity_type level, char const * func, char const * file, unsigned int line, std::type_info const & type)
        {
            return loggable_line_logger(*this, new detail::loggable_buffer<char_type>(level, func, file, line, type));
        }

        template <typename Buffer>
        void write(std::unique_ptr<Buffer> && buf) const
        {
            string_type dest;
            dest.reserve(1024);

            boost::iostreams::stream_buffer< boost::iostreams::back_insert_device<string_type> > strbuf(dest);
            std::basic_ostream<char_type> os(&strbuf);
            buf->format(os, format_);
            os.flush();

            std::cout << dest << std::endl;
            //for(auto const & sink : sinks_) {
            //    sink->write(dest.c_str(), dest.size());
            //}
        }

        void set_format(string_type const & format)
        {
            format_ = format;
        }

        string_type const & get_format() const
        {
            return format_;
        }

    private:
        std::string name_;
        string_type format_ = "%F %T.%U [%L] - %v";
        std::vector< std::unique_ptr< sinks::logger_sink<char_type, mutex_type> > > sinks_;
    };

    template <typename Tag>
    static logger & get()
    {
        return singleton::get_mutable_instance().find_or_construct(std::type_index(typeid(Tag)));
    }

    static logger & get_default()
    {
        return get<default_tag>();
    }

    static std::string const & project_name()
    {
        return singleton::get_const_instance().name_;
    }

    static void project_name(std::string const & name)
    {
        singleton::get_mutable_instance().name_ = name;
    }

private:
    logger & find_or_construct(std::type_index const & idx)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        auto it = loggers_.find(idx);
        if (it == loggers_.end()) {
            it = loggers_.emplace_hint(it, idx, *this);
        }
        return it->second;
    }

private:
    // logger は削除できない
    std::unordered_map<std::type_index, logger> loggers_;
    std::string name_;
    mutable mutex_type mutex_;
};

} }


#ifndef ACQUA_LOG_trace
#define ACQUA_LOG_trace    detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::trace, __func__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_debug
#define ACQUA_LOG_debug    detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::debug, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_info
#define ACQUA_LOG_info     detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::info, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_notice
#define ACQUA_LOG_notice   detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::notice, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_warning
#define ACQUA_LOG_warning  detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::warning, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_error
#define ACQUA_LOG_error    detail__logging(acqua::log::koenig_lookup_tag(), acqua::log::error, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_critical
#define ACQUA_LOG_critical detail__logging(acqua::log::koenig_lookup_tag(), acqua::log::critical, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_alert
#define ACQUA_LOG_alert    detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::alert, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif

#ifndef ACQUA_LOG_emerg
#define ACQUA_LOG_emerg    detail_logging(acqua::log::koenig_lookup_tag(), acqua::log::emerg, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#endif


#ifndef LOG
#define LOG(level) ACQUA_LOG_ ## level
#endif
