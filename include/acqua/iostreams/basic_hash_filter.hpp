#pragma once

#include <array>
#include <boost/system/error_code.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>

namespace acqua { namespace iostreams {

template <typename Context>
class basic_hash_filter
    : private Context
{
public:
    using char_type = char;
    static constexpr std::size_t buffer_size = Context::buffer_size;
    struct category : boost::iostreams::multichar_dual_use_filter_tag, boost::iostreams::closable_tag {};

    template <typename CharT, std::size_t N>
    explicit basic_hash_filter(std::array<CharT, N> & buffer)
        : Context(reinterpret_cast<unsigned char *>(buffer.data()))
    {
        static_assert(sizeof(CharT) == 1, "");
        static_assert(N >= buffer_size, "");
        Context::init(error_);
    }

    template <typename CharT, std::size_t N>
    explicit basic_hash_filter(CharT (&buffer)[N])
        : Context(reinterpret_cast<unsigned char *>(buffer))
    {
        static_assert(sizeof(CharT) == 1, "");
        static_assert(N >= buffer_size, "");
        Context::init(error_);
    }

    template <typename Source>
    std::streamsize read(Source & src, char * s, std::streamsize n)
    {
        n = boost::iostreams::read(src, s, n);
        if (n > 0)
            Context::update(s, static_cast<std::size_t>(n), error_);
        return n;
    }

    template <typename Sink>
    std::streamsize write(Sink & sink, char const * s, std::streamsize n)
    {
        if (n > 0)
            Context::update(s, static_cast<std::size_t>(n), error_);
        return boost::iostreams::write(sink, s, n);
    }

    template <typename Device>
    void close(Device &, std::ios_base::openmode)
    {
        Context::finish(error_);
    }

private:
    boost::system::error_code error_;
};

} }
