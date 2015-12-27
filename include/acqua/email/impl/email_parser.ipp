#pragma once

#include <iostream>
#include <algorithm>
#include <acqua/email/email_parser.hpp>

namespace acqua { namespace email {

template <typename String>
class basic_email_parser<String>::impl
{
public:
    explicit impl(boost::system::error_code & error, basic_email<String> & email)
        : error_(error), email_(email) {}

    void parse_line(std::string const & line)
    {
        if (line == ".") {
            in_progress = false;
            return;
        }
        std::cout << line << std::endl;
    }

private:
    boost::system::error_code & error_;
    basic_email<String> & email_;

public:
    bool in_progress = true;
};

template <typename String>
inline basic_email_parser<String>::basic_email_parser(basic_email<String> & email)
    : impl_(new impl(error_, email)) {}

template <typename String>
inline std::streamsize basic_email_parser<String>::write(char_type const * beg, std::streamsize size)
{
    if (size == EOF || !impl_->in_progress)
        return EOF;

    if (size <= 0)
        return size;

    char sep[] = { '\r', '\n' };
    char const * end = beg + size;
    char const * it;

    // 前回の最後が '\r' のときで 今回の最初が '\n' のときは '\n' を飛ばす
    if (last_ == '\r' && *beg == '\n')
        ++beg;
    last_ = end[-1];

    for(; (it = std::find_first_of(beg, end, sep, sep+2)) != end; beg = it) {
        line_.append(beg, it);  // 改行コードは含まない
        impl_->parse_line(line_);
        line_.clear();
        if (*it == '\r')
            ++it;
        if (it == end)
            return size;
        if (*it == '\n')
            ++it;
        if (it == end)
            return size;
    }

    // TODO: line_ は RFC に則った許容量を超えたときにエラーにしたほうがいいかも

    // 残り
    line_.append(beg, end);
    return size;
}

} }
