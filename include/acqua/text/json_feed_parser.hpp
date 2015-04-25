/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/text/json_feed/json_parser.hpp>

namespace acqua { namespace text {

template <typename Destinate, typename CharT>
class json_feed_parser
{
    using data_type = json_feed::json_parser<Destinate, CharT>;

public:
    json_feed_parser(Destinate & dest)
        : data_(dest) {}

    ~json_feed_parser()
    {
        if (good()) {
            try {
                *this << '\0';
            } catch(...) {}
        }
    }
    
    bool good() const
    {
        return data_.is_progress();
    }
    
    json_feed_parser & operator<<(CharT ch)
    {
        while(!data_.parse(ch));
        return *this;
    }

    json_feed_parser & operator<<(CharT const * str)
    {
        while(*str)
            *this << *str++;
        return *this;
    }

    json_feed_parser & operator<<(std::basic_string<CharT> const & str)
    {
        for(CharT const & ch : str)
            *this << ch;
        return *this;
    }

private:
    data_type data_;
};

} }
