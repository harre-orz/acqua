#pragma once

#include <iostream>
#include <locale>
#include <boost/system/error_code.hpp>
#include <acqua/text/mime_header.hpp>
#include <acqua/text/base64.hpp>
#include <acqua/text/email_message.hpp>
#include <acqua/text/email_encoding.hpp>

namespace acqua { namespace text {

namespace email_impl {

class head_parser
{
public:
    explicit head_parser()
        : boundary_(nullptr) {}

    explicit head_parser(std::string const & boundary)
        : boundary_(&boundary) {}

    template <typename Parser>
    bool operator()(std::string const & line, Parser & next)
    {
        auto it = line.begin();
        if (line.empty()) {
            add_header_param(next);
            next.body(boundary_);
            return true;
        } else if (line.size() == 1 && line[0] == '.') {
            add_header_param(next);
            next.complete();
            return true;;
        } else if (!std::isspace(*it, std::locale::classic())) {
            // 次のヘッダー
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダーに不備がある場合は、ヘッダー解析終了
                next.body(boundary_);
                return false;
            }
            add_header_param(next);
            buffer_.clear();
            name_.assign(line.begin(), it);
        }

        while(std::isspace(*++it, std::locale::classic()))
            ;
        if (!buffer_.empty()) buffer_.push_back(' ');
        buffer_.append(it, line.end());
        return true;
    }

    template <typename Parser>
    void add_header_param(Parser & next) const
    {
        if (!name_.empty() && !buffer_.empty()) {
            auto & disp = next.email[name_];
            if (boost::istarts_with(name_, "Content-")) {
                acqua::text::mime_header::decode(buffer_.begin(), buffer_.end(), disp.str(), disp);
            } else {
                acqua::text::mime_header::decode(buffer_.begin(), buffer_.end(), disp.str());
            }
        }
    }

private:
    std::string const * boundary_;
    std::string name_;
    std::string buffer_;
};

template <typename Decoding, typename Parser>
class body_parser
{
public:
    explicit body_parser(Decoding dec, std::string const * child_boundary = nullptr, std::string const * parent_boundary = nullptr)
        : dec_(dec), child_boundary_(child_boundary), parent_boundary_(parent_boundary) {}

    bool operator()(std::string const & line, Parser & next)
    {
        if (line.size() == 1 && line[0] == '.') {
            dec_.flush(next.email);
            next.complete();
        } else if (is_multipart_begin(line)) {
            multipart_.reset(new Parser(next, *child_boundary_));
        } else if (is_multipart_end(line)) {
            dec_.flush(next.email);
        } else if (multipart_ && !multipart_->is_complete()) {
            multipart_->parse(line);
        } else {
            dec_.write(next.email, line);
        }

        return true;
    }

private:
    bool is_multipart_begin(std::string const & line) const
    {
        return (child_boundary_ && child_boundary_->size() == line.size() + 2 &&
                line[0] == '-' && line[1] == '-' &&
                std::equal(line.begin()+2, line.end(), child_boundary_->begin()));
    }

    bool is_multipart_end(std::string const & line) const
    {
        return (parent_boundary_ && parent_boundary_->size() == line.size() + 4 &&
                line[0] == '-' && line[1] == '-' && line[2] == '-' && line[3] == '-' &&
                std::equal(line.begin()+2, line.end()-2, child_boundary_->begin()));
    }

private:
    std::unique_ptr<Parser> multipart_;
    Decoding dec_;
    std::string const * child_boundary_;
    std::string const * parent_boundary_;
};

template <typename Email>
class feed_parser : public boost::variant< head_parser,
                                           body_parser<no_decoding, feed_parser<Email> >,
                                           body_parser<ascii_decoding, feed_parser<Email> >,
                                           body_parser<quoted_decoding, feed_parser<Email> >,
                                           body_parser<base64_message_decoding, feed_parser<Email> >,
                                           body_parser<base64_binary_decoding, feed_parser<Email> > >
{
    using base_type = boost::variant< head_parser,
                                      body_parser<no_decoding, feed_parser<Email> >,
                                      body_parser<ascii_decoding, feed_parser<Email> >,
                                      body_parser<quoted_decoding, feed_parser<Email> >,
                                      body_parser<base64_message_decoding, feed_parser<Email> >,
                                      body_parser<base64_binary_decoding, feed_parser<Email> > >;

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

    explicit feed_parser(feed_parser & parent, std::string const & parent_boundary)
        : base_type(head_parser(parent_boundary)), error_(parent.error_), email(parent.email) {}

    bool is_complete() const
    {
        return is_compl_;
    }

    void complete()
    {
        is_compl_ = true;
    }

    void body(std::string const * parent_boundary)
    {
        std::string const * child_boundary = nullptr;  // バウンダリ
        std::string charset = "us-ascii";
        bool is_binary = true;  // 改行コードを変換しないか？

        auto it = email.find("Content-Type");
        if (it != email.end()) {
            is_binary = !boost::algorithm::istarts_with(it->second.str(), "text/");
            auto it2 = it->second.find("charset");
            if (it2 != it->second.end())
                charset = it2->second;
            it2 = it->second.find("boundary");
            if (it2 != it->second.end()) {
                child_boundary = &it2->second;
            }
        }

        it = email.find("Content-Transfer-Encoding");
        if (it != email.end()) {
            if (boost::algorithm::iequals(it->second.str(), "base64")) {
                if (is_binary) {
                    *static_cast<base_type *>(this) = body_parser<base64_binary_decoding, feed_parser<Email> >(
                        base64_binary_decoding(), child_boundary, parent_boundary
                    );
                } else {
                    *static_cast<base_type *>(this) = body_parser<base64_message_decoding, feed_parser<Email> >(
                        base64_message_decoding(charset), child_boundary, parent_boundary
                    );
                }
                return;
            }

            if (boost::algorithm::iequals(it->second.str(), "quoted-printable")) {
                *static_cast<base_type *>(this) = body_parser<quoted_decoding, feed_parser<Email> >(
                    quoted_decoding(charset), child_boundary, parent_boundary
                );
                return;
            }
        }

        if (is_binary) {
            *static_cast<base_type *>(this) = body_parser<no_decoding, feed_parser<Email> >(
                no_decoding(), child_boundary, parent_boundary
            );
        } else {
            *static_cast<base_type *>(this) = body_parser<ascii_decoding, feed_parser<Email> >(
                ascii_decoding(charset), child_boundary, parent_boundary
            );
        }
        return;
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
