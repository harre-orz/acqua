#pragma once

#include <acqua/string_cast.hpp>

namespace acqua { namespace email { namespace detail {

/*!
  何もしないエンコーダ.
 */
class noop_encoder
{
public:
    template <typename Sink, typename String>
    void write(Sink & sink, String const & line)
    {
        sink << acqua::string_cast(line) << '\r' << '\n';
    }

    template <typename Sink>
    void flush(Sink & sink)
    {
    }
};


/*!
  何もしないデコーダ.
 */
class noop_decoder
{
public:
    template <typename Sink>
    void write(Sink & sink, std::string const & line)
    {
        sink << acqua::string_cast(line) << std::endl;
    }

    template <typename Sink>
    void flush(Sink &)
    {
    }
};

} } }
