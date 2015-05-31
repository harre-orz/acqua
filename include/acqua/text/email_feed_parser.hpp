#pragma once

#include <iostream>
#include <locale>
#include <boost/system/error_code.hpp>
#include <acqua/text/encoding.hpp>
#include <acqua/text/email_header.hpp>
#include <acqua/text/email_message.hpp>

namespace acqua { namespace text {

namespace email_impl {

template <typename String>
class head_parser
{
public:
    explicit head_parser(String const & boundary = String())
        : boundary_(boundary)
    {}

    template <typename Parser>
    bool operator()(String const & line, Parser & next)
    {
        if (line.empty()) {
            next.body();
        } else if (!std::isspace(line[0], std::locale::classic())) {
            auto pos = line.find(':');
            if (pos == line.npos) {
                next.body();
                return false;
            }

            name_.assign(line.c_str(), pos);
            while(std::isspace(line[++pos], std::locale::classic()));
            next.email.header(name_).append(line.begin()+pos, line.end());
        } else if (!name_.empty()) {
            next.email.header(name_).append(line);
        } else {
            next.body(ascii_decoding("UTF-8"), "hello", boundary_);
            return false;
        }

        return true;
    }

private:
    String name_;
    String boundary_;
};


template <typename String, typename Decoding, typename Parser>
class body_parser
{
public:
    explicit body_parser(Decoding decoding, String const & multipart_boundary, String const & parent_boundary)
        : decoding_(decoding), multipart_boundary_(multipart_boundary), parent_boundary_(parent_boundary) {}

    bool operator()(String const & line, Parser & next)
    {
        if (in_multipart_ && !in_multipart_->is_complete()) {
            in_multipart_->parse(line);
        } else if (is_boundary_begin(line, multipart_boundary_)) {
            in_multipart_.reset(new Parser(next, multipart_boundary_));
        } else if (is_boundary_end(line, parent_boundary_)) {
            decoding_.flush(std::cout);
            next.complete();
        } else {
            decoding_.write(next.email, line);
        }

        return true;
    }

private:
    static bool is_boundary_begin(String const & line, String const & boundary)
    {
        return (boundary.size() && line.size() == boundary.size()+2 &&
                line[0] == '-' && line[1] == '-' &&
                std::equal(line.begin()+2, line.end(), boundary.begin()));
    }

    static bool is_boundary_end(String const & line, String const & boundary)
    {
        return (boundary.size() && line.size() == boundary.size()+4 &&
                line[0] == '-' && line[1] == '-' && line[line.size()-1] == '-' && line[line.size()-2] == '-' &&
                std::equal(line.begin()+2, line.end()-2, boundary.begin()));
    }

private:
    Decoding decoding_;
    String multipart_boundary_;
    String parent_boundary_;
    std::unique_ptr<Parser> in_multipart_;
};


template <typename String>
class feed_parser
    : public boost::variant< head_parser<String>,
                             body_parser<String, no_decoding, feed_parser<String> >,
                             body_parser<String, ascii_decoding, feed_parser<String> >,
                             body_parser<String, quoted_decoding, feed_parser<String> >,
                             body_parser<String, base64_file_decoding, feed_parser<String> >,
                             body_parser<String, base64_text_decoding, feed_parser<String> >
                             >
{
    using base_type = boost::variant<
        head_parser<String>,
        body_parser<String, no_decoding, feed_parser<String> >,
        body_parser<String, ascii_decoding, feed_parser<String> >,
        body_parser<String, quoted_decoding, feed_parser<String> >,
        body_parser<String, base64_file_decoding, feed_parser<String> >,
        body_parser<String, base64_text_decoding, feed_parser<String> >
        >;

    struct feed_visitor : boost::static_visitor<>
    {
        String const & line_;
        feed_parser & next_;
        explicit feed_visitor(String const & line, feed_parser & next)
            : line_(line), next_(next) {}

        template <typename Parser>
        void operator()(Parser & parser) const
        {
            if (line_.size() == 0 && line_[0] == '.')
                next_.complete();
            else
                while(!parser(line_, next_));
        }
    };

    using email_type = email_message<String>;

public:
    feed_parser(email_type & email)
        : base_type(head_parser<String>(String()))
        , email(email) {}

    feed_parser(feed_parser & parent, String const & str)
        : base_type(head_parser<String>(str))
        , email(parent.email) {}

    bool is_complete() const
    {
        return is_compl_;
    }

    void body()
    {
        *static_cast<base_type *>(this) = body_parser<String, no_decoding, feed_parser>(no_decoding(), "", "");
    }

    template <typename Decoding>
    void body(Decoding decoding, String const & multipart, String const & parent)
    {
        *static_cast<base_type *>(this) = body_parser<String, Decoding, feed_parser>(decoding, multipart, parent);
    }

    void complete()
    {
        is_compl_ = true;
    }

    void parse(String const & line)
    {
        boost::apply_visitor(feed_visitor(line, *this), *this);
    }

    email_type & email;
private:
    bool is_compl_;
};

}  // email_impl

template <typename String>
class email_feed_parser
{
public:
    using value_type = String;
    using char_type = typename String::value_type;
    using size_type = typename String::size_type;

public:
    template <typename Email>
    explicit email_feed_parser(Email & email)
        : parser_(email) {}

    bool is_complete() const
    {
        return false;
    }

    boost::system::error_code const & get_error_code() const
    {
        return error_;
    }

    email_feed_parser & parse(char_type const * beg, size_type size)
    {
        if (size > 0) {
            if (prev_ == '\r' && *beg == '\n')
                ++beg;
            char_type const * end = beg + size;
            prev_ = end[-1];

            do {
                constexpr char crln[] = { '\r', '\n' };
                auto pos = std::find_first_of(beg, end, crln, crln + sizeof(crln)/sizeof(crln[0]));
                line_.append(beg, pos);
                if (pos < end) {
                    parser_.parse(line_);
                    line_.clear();
                    if (*pos == '\r')
                        ++pos;
                    if (pos < end && *pos == '\n')
                        ++pos;
                }
                beg = pos;
            } while(beg < end);
        }

        return *this;
    }

private:
    boost::system::error_code error_;
    email_impl::feed_parser<value_type> parser_;
    value_type line_;
    char_type prev_ = '\0';
};

} }
