/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>

namespace acqua { namespace email {

template <
    typename EMail
    >
class feed_parser
{
    class impl;

public:
    feed_parser(EMail & email)
        : impl_(new impl(error_, email))
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

} }

#include <acqua/email/impl/feed_parser_impl.ipp>
