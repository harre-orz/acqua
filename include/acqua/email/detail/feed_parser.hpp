/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <memory>
#include <boost/system/error_code.hpp>

namespace acqua { namespace email { namespace detail {

template <typename Mail>
class feed_parser
{
    class impl;

public:
    using char_type = typename Mail::char_type;
    using traits_type = typename Mail::traits_type;

public:
    feed_parser(Mail & mail)
        : impl_(new impl(error_, mail))
    {
    }

    bool is_terminated() const
    {
        return (bool)error_ || impl_->is_terminated();
    }

    boost::system::error_code const & get_error_code() const
    {
        return error_;
    }

    feed_parser & parse(char ch)
    {
        if (ch == '\r' || ch == '\n') {
            if (prev_ != '\r') {
                impl_->do_parse_line(line_);
                line_.clear();
            }
        } else {
            line_.push_back(ch);
        }
        prev_ = ch;
        return *this;
    }

    friend std::istream & operator>>(std::istream & is, feed_parser & rhs)
    {
        char ch;
        while(!rhs.is_terminated() && is.get(ch))
            rhs.parse(ch);
        return is;
    }

private:
    boost::system::error_code error_;
    std::unique_ptr<impl> impl_;
    std::string line_;
    char prev_ = '\0';
};

} } }

#include <acqua/email/detail/impl/feed_parser_impl.ipp>
