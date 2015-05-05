#pragma once

#include <utility>
#include <memory>
#include <type_traits>

namespace acqua { namespace log { namespace detail {

//! マニピュレータタグ
struct manip_tag {};

template <typename Logger, typename Buffer>
class line_logger
{
public:
    explicit line_logger(Logger & logger, Buffer * buf)
        : logger_(logger), buffer_(buf) {}

    line_logger(line_logger const &) = delete;

    line_logger(line_logger && rhs)
        : logger_(rhs.logger_), buffer_(std::move(rhs.buffer_)) {}

    ~line_logger()
    {
        if (buffer_) {
            try {
                logger_.write(std::move(buffer_));
            } catch(...) {}
        }
    }

    template <typename T, typename std::enable_if<!std::is_base_of<manip_tag, T>::value>::type * = nullptr>
    line_logger const & operator<<(T const & t) const
    {
        *buffer_ << t;
        return *this;
    }

private:
    Logger & logger_;
    std::unique_ptr<Buffer> buffer_;
};


} } }
