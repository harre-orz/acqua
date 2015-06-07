#pragma once

#include <iostream>
#include <locale>
#include <boost/system/error_code.hpp>
#include <acqua/text/mime_header.hpp>
#include <acqua/text/base64.hpp>
#include <acqua/text/encoding.hpp>
#include <acqua/text/email_header.hpp>
#include <acqua/text/email_message.hpp>

namespace acqua { namespace text {

namespace email_impl {

class no_boundary
{
};

template <typename Boundary>
class head_parser
{
public:
    template <typename Parser>
    bool operator()(std::string const & line, Parser & next)
    {
        auto it = line.begin();
        if (line.empty()) {
            apply(next);
            next.body();
            return true;
        } else if (line.size() == 1 && line[0] == '.') {
            apply(next);
            next.complete();
            return true;;
        } else if (!std::isspace(*it, std::locale::classic())) {
            // 次のヘッダー
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダーに不備がある場合は、ヘッダー解析終了
                next.body();
                return false;
            }
            apply(next);
            buffer_.clear();
            name_.assign(line.begin(), it);
        }

        while(std::isspace(*++it, std::locale::classic()));
        if (!buffer_.empty()) buffer_.push_back(' ');
        buffer_.append(it, line.end());
        return true;
    }

    template <typename Parser>
    void apply(Parser & next) const
    {
        if (!name_.empty() && !buffer_.empty()) {
            auto & pair = next.email.header.insert(name_);
            if (boost::istarts_with(name_, "Content-")) {
                acqua::text::mime_header::decode(buffer_.begin(), buffer_.end(), pair.first, pair.second);
            } else {
                acqua::text::mime_header::decode(buffer_.begin(), buffer_.end(), pair.first);
            }
        }
    }

private:
    std::string name_;
    std::string buffer_;
};


template <typename Email>
class feed_parser : public boost::variant< head_parser<no_boundary> >
{
    struct feed_visitor : boost::static_visitor<>
    {
        std::string const & line_;
        feed_parser & next_;

        explicit feed_visitor(std::string const & line, feed_parser & next)
            : line_(line), next_(next) {}

        template <typename Parser>
        void operator()(Parser & parser) const
        {
            while(!parser(line_, next_));
        }
    };

public:
    explicit feed_parser(boost::system::error_code & error, Email & email)
        : error_(error), email(email) {}

    bool is_complete() const
    {
        return is_compl_;
    }

    void complete()
    {
        is_compl_ = true;
    }

    void body()
    {
        complete();
    }

    void parse(std::string const & line)
    {
        boost::apply_visitor(feed_visitor(line, *this), *this);
    }

private:
    bool is_compl_ = false;
    boost::system::error_code & error_;

public:
    Email & email;
};

}  // email_impl

template <typename Email>
class email_feed_parser
{
private:
    using parser_type = email_impl::feed_parser<Email>;

public:
    explicit email_feed_parser(Email & email)
        : parser_(error_, email) {}

    bool is_complete() const
    {
        return error_ || parser_.is_complete();
    }

    boost::system::error_code const & get_error_code() const
    {
        return error_;
    }

    email_feed_parser & parse(char ch)
    {
        if (ch == '\r' || ch == '\n') {
            if (prev_ != '\r') {
                parser_.parse(line_);
                line_.clear();
            }
        } else {
            line_.push_back(ch);
        }
        prev_ = ch;
        return *this;
    }

    friend std::istream & operator>>(std::istream & is, email_feed_parser & rhs)
    {
        char ch;
        while(!rhs.is_complete() && is.get(ch))
            rhs.parse(ch);
        return is;
    }

private:
    boost::system::error_code error_;
    parser_type parser_;
    std::string line_;
    char prev_ = '\0';
};

} }
