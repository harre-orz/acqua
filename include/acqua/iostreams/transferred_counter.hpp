/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/categories.hpp>

namespace acqua { namespace iostreams {

template <typename CharT>
class basic_transferred_counter
{
public:
    using char_type = CharT;
    struct category : boost::iostreams::multichar_dual_use_filter_tag {};

public:
    explicit basic_transferred_counter(std::size_t & size)
        : size_(size)
    {
        size_ = 0;
    }

    template <typename Source>
    std::streamsize read(Source & src, char * s, std::streamsize n)
    {
        n = boost::iostreams::read(src, s, n);
        if (n > 0)
            size_ += static_cast<std::size_t>(n);
        return n;
    }

    template <typename Sink>
    std::streamsize write(Sink & sink, char const * s, std::streamsize n)
    {
        if (n > 0)
            size_ += static_cast<std::size_t>(n);
        return boost::iostreams::write(sink, s, n);
    }

private:
    std::size_t & size_;
};

using transferred_counter = basic_transferred_counter<char>;
using wtransferred_counter = basic_transferred_counter<wchar_t>;

} }
