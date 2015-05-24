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
#include <acqua/log/sinks/console_sink.hpp>

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
        : default_logger_(new logger())
        , cout_sink_(std::cout)
        , cerr_sink_(std::cerr)
    {
        default_logger_->push(cout_sink_);
    }

public:
    //! デフォルトのロガータグ
    struct default_tag {};

    class logger;

    using logging_line_logger = detail::line_logger<logger, detail::logging_buffer<char_type> >;
    using loggable_line_logger = detail::line_logger<logger,  detail::loggable_buffer<char_type> >;

    //! ロガー
    class logger
        : public std::enable_shared_from_this<logger>
    {
    public:
        using char_type = core::char_type;
        using mutex_type = core::mutex_type;
        using string_type = std::basic_string<char_type>;

        // template <typename Sink>
        // void push(Sink const & sink)
        // {
        //     // TODO: スレッドセーフでない
        //     sinks_.emplace_back(new Sink(sink));
        // }

        void push(sinks::console_sink<char_type, mutex_type> & sink)
        {
            auto it = std::find(sinks_.begin(), sinks_.end(), &sink);
            if (it == sinks_.end())
                sinks_.emplace_back(&sink);
        }

        void pop()
        {
            auto core_ = &singleton::get_mutable_instance();

            if (!sinks_.empty()) {
                auto * sink = sinks_.back();
                sinks_.pop_back();
                if (sink != &core_->cout_sink_ || sink != &core_->cerr_sink_)
                    delete sink;
            }
        }

        template <typename Tag>
        void alias()
        {
            singleton::get_mutable_instance().alias((Tag *)(nullptr), this->shared_from_this());
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

            for(auto const & sink : sinks_) {
               sink->write(dest.c_str(), dest.size());
            }
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
        string_type format_ = "%F %T.%U [%L] - %v";
        std::vector<sinks::logger_sink<char_type, mutex_type> *> sinks_;
    };

    template <typename Tag>
    static std::shared_ptr<logger> get()
    {
        return singleton::get_mutable_instance().find_or_construct((Tag *)(nullptr));
    }

    static std::shared_ptr<logger> get_default()
    {
        return get<default_tag>();
    }

    template <typename Tag>
    static bool remove()
    {
        return singleton::get_mutable_instance().remove((Tag *)(nullptr));
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
    template <typename Tag>
    std::shared_ptr<logger> find_or_construct(Tag const *)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        auto it = loggers_.find(std::type_index(typeid(Tag)));
        if (it == loggers_.end())
            it = loggers_.emplace_hint(it, std::type_index(typeid(Tag)), std::shared_ptr<logger>(new logger()));
        return it->second;
    }

    std::shared_ptr<logger> find_or_construct(default_tag const *)
    {
        return default_logger_;
    }

    template <typename Tag>
    bool remove(Tag *)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        auto it = loggers_.find(std::type_index(typeid(Tag)));
        if (it == loggers_.end())
            return false;
        loggers_.erase(it);
        return true;
    }

    bool remove(default_tag *)
    {
        return false;
    }

    template <typename Tag>
    void alias(Tag const *, std::shared_ptr<logger> that)
    {
        std::lock_guard<decltype(mutex_)> lock(mutex_);

        auto it = loggers_.find(std::type_index(typeid(Tag)));
        if (it != loggers_.end() && it->second != that)
            it->second = that;
        else
            loggers_.emplace_hint(it, std::type_index(typeid(Tag)), that);
    }

    void alias(default_tag *, std::shared_ptr<logger> that)
    {
        if (default_logger_ != that)
            default_logger_ = that;
    }

private:
    std::shared_ptr<logger> default_logger_;
    sinks::console_sink<char_type, mutex_type> cout_sink_;
    sinks::console_sink<char_type, mutex_type> cerr_sink_;
    std::unordered_map<std::type_index, std::shared_ptr<logger> > loggers_;
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
