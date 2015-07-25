#pragma once

#include <iostream>
#include <locale>
#include <boost/system/error_code.hpp>
#include <acqua/email/basic_message.hpp>
#include <acqua/email/detail/literals.hpp>
#include <acqua/email/detail/feed_parser_state.hpp>

namespace acqua { namespace email {

template <
    typename EMail,
    typename Literals = acqua::email::detail::literals<typename EMail::char_type>
    >
class feed_parser
{
private:
    using state_type = acqua::email::detail::feed_parser_state<EMail, Literals>;

public:
    explicit feed_parser(EMail & email)
        : state_(error_, email) {}

    bool is_terminated() const
    {
        return error_ || state_.is_terminated();
    }

    boost::system::error_code const & get_error_code() const
    {
        return error_;
    }

    feed_parser & parse(char ch)
    {
        if (ch == '\r' || ch == '\n') {
            if (prev_ != '\r') {
                state_.do_parse_oneline(line_);
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
    state_type state_;
    std::string line_;
    char prev_ = '\0';
};

} }
