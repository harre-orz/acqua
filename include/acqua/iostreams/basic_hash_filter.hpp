/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <array>
#include <memory>
#include <cstring>

namespace acqua { namespace iostreams {

template <typename Context, std::size_t BufferSize>
class basic_hash_filter
{
public:
    using char_type = char;
    struct category : boost::iostreams::multichar_dual_use_filter_tag, boost::iostreams::closable_tag {};
    static constexpr std::size_t buffer_size = BufferSize;

private:
    explicit basic_hash_filter(unsigned char * buffer)
        : impl_(new Context{})
        , buffer_(buffer)
    {
        boost::system::error_code ec;
        impl_->init(ec);
        if (ec) throw boost::system::system_error(ec, "init");
    }

public:
    template <typename CharT, std::size_t N>
    explicit basic_hash_filter(std::array<CharT, N> & buffer)
        : basic_hash_filter(reinterpret_cast<unsigned char *>(buffer.data()))
    {
        static_assert(sizeof(CharT) == 1, "CharT must be 1byte.");
        static_assert(N >= buffer_size, "N must be longer buffer_size.");
    }

    template <typename CharT, std::size_t N>
    explicit basic_hash_filter(CharT (&buffer)[N])
        : basic_hash_filter(reinterpret_cast<unsigned char *>(buffer))
    {
        static_assert(sizeof(CharT) == 1, "CharT must be 1byte.");
        static_assert(N >= buffer_size, "N must be longer buffer_size.");
    }

    template <typename Source>
    std::streamsize read(Source & src, char * s, std::streamsize n)
    {
        n = boost::iostreams::read(src, s, n);
        if (n > 0) {
            boost::system::error_code ec;
            impl_->update(s, static_cast<std::size_t>(n), ec);
            if (ec) throw boost::system::system_error(ec, "read");
        }
        return n;
    }

    template <typename Sink>
    std::streamsize write(Sink & sink, char const * s, std::streamsize n)
    {
        if (n > 0) {
            boost::system::error_code ec;
            impl_->update(s, static_cast<std::size_t>(n), ec);
            if (ec) throw boost::system::system_error(ec, "write");
        }
        return boost::iostreams::write(sink, s, n);
    }

    template <typename Device>
    void close(Device &, std::ios_base::openmode) noexcept
    {
        boost::system::error_code ec;
        std::memset(buffer_, 0, buffer_size);
        impl_->finish(buffer_, buffer_size, ec);
        if (ec) throw boost::system::system_error(ec, "close");
    }

private:
    std::shared_ptr<Context> impl_;
    unsigned char * buffer_;
};

} }
