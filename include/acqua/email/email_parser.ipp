#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/email/email_parser.hpp>
#include <acqua/email/decode_mimeheader.hpp>
#include <acqua/email/message.hpp>
#include <acqua/email/detail/encoding.hpp>
#include <acqua/iostreams/ostream_codecvt.hpp>
#include <acqua/iostreams/ascii_filter.hpp>
#include <acqua/iostreams/qprint_filter.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/variant.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/scope_exit.hpp>
#include <iostream>
#include <algorithm>

namespace acqua { namespace email { namespace detail {

template <typename Message>
struct parser_impl;

class header_parser
{
public:
    header_parser(std::string const * boundary)
        : boundary_(boundary) {}

    template <typename Impl>
    bool operator()(std::string const & line, Impl & impl)
    {
        if (line.empty()) {
            // ヘッダー終了
            append(impl.email);
            impl.to_payload(boundary_);
            return true;
        }

        if (line.size() == 1 && line[0] == '.') {
            // メールの終了記号のときは、ヘッダーの解析を打ち切り line を本文として処理させる
            append(impl.email);
            impl.to_payload(boundary_);
            return false;
        }

        auto it = line.begin();
        if (!std::isspace(*it, std::locale::classic())) {
            // 新しいヘッダー
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダー名と値を分けられないときは、ヘッダーの解析を打ち切り line を本文として処理させる
                impl.to_payload(boundary_);
                return false;
            }

            append(impl.email);
            name_.assign(line.begin(), it);
            params_.clear();
        }

        while(std::isspace(*++it, std::locale::classic()))
            ;
        if (!params_.empty())
            params_.push_back(' ');
        params_.append(it, line.end());
        return true;
    }

private:
    template <typename String>
    void append(basic_message<String> & mes)
    {
        if (!name_.empty() && !params_.empty()) {
            auto & disp = mes.headers[name_];
            if (boost::istarts_with(name_, "Content-")) {
                decode_mimeheader(params_.begin(), params_.end(), disp.str(), disp);
            } else {
                decode_mimeheader(params_.begin(), params_.end(), disp.str());
            }
        }
    }

private:
    std::string const * boundary_;
    std::string name_;
    std::string params_;
};


template <typename Message>
class payload_parser
{
    using char_type = typename Message::char_type;
    using traits_type = typename Message::traits_type;
    using streambuf_type = std::basic_streambuf<char_type, traits_type>;

public:
    // 文字コード変換をしない
    explicit payload_parser(std::string const * child_boundary, std::string const * parent_boundary,
                            Message & email, encoding enc)
        : child_boundary_(child_boundary), parent_boundary_(parent_boundary)
        , out_(new boost::iostreams::filtering_ostream)
    {
        switch(enc) {
            case encoding::ascii:
                break;
            case encoding::qprint:
                out_->push(acqua::iostreams::qprint_decoder());
                break;
            case encoding::base64:
                out_->push(acqua::iostreams::base64_decoder());
                break;
        }
        out_->push(acqua::iostreams::ostream_code_converter<char_type>(static_cast<streambuf_type *>(email)));
    }

    // 文字コード変換をする
    explicit payload_parser(std::string const * child_boundary, std::string const * parent_boundary,
                                 Message & email, encoding enc, std::string const & charset)
        : child_boundary_(child_boundary), parent_boundary_(parent_boundary)
        , out_(new boost::iostreams::filtering_ostream)
    {
        switch(enc) {
            case encoding::ascii:
                out_->push(acqua::iostreams::ascii_decoder());
                break;
            case encoding::qprint:
                out_->push(acqua::iostreams::qprint_decoder());
                break;
            case encoding::base64:
                out_->push(acqua::iostreams::base64_decoder());
                break;
        }
        out_->push(acqua::iostreams::ostream_code_converter<char_type>(static_cast<streambuf_type *>(email), charset));
    }

    payload_parser(payload_parser const &) = delete;
    payload_parser(payload_parser && rhs) noexcept
        : child_boundary_(rhs.child_boundary_), parent_boundary_(rhs.parent_boundary_)
        , out_(std::move(rhs.out_)), subpart_(std::move(rhs.subpart_))
    {
        rhs.out_ = nullptr;
        rhs.subpart_ = nullptr;
    }

    payload_parser & operator=(payload_parser const &) = delete;
    payload_parser & operator=(payload_parser && rhs) noexcept
    {
        child_boundary_ = rhs.child_boundary_;
        parent_boundary_ = rhs.parent_boundary_;
        out_ = std::move(rhs.out_);
        subpart_ = std::move(rhs.subpart_);
        rhs.out_ = nullptr;
        rhs.subpart_ = nullptr;
        return *this;
    }

    ~payload_parser() noexcept
    {
        delete subpart_;
        delete out_;
    }

    template <typename Impl>
    bool operator()(std::string const & line, Impl & impl)
    {
        if (line.size() == 1 && line[0] == '.') {
            impl.complete();
        } else if (this->is_child_multipart_begin(line)) {
            impl.complete();
        } else if (this->is_parent_multipart_end(line)) {
            impl.to_subpart(this->child_boundary_, subpart_);
        } else if (subpart_ && subpart_->in_progress) {
            subpart_->parse_line(line);
        } else {
            *out_ << line;
        }
        return true;
    }

private:
    bool is_child_multipart_begin(std::string const & line) const
    {
        return (child_boundary_ && child_boundary_->size() + 2 == line.size() &&
                line[0] == '-' && line[1] == '-' &&
                std::equal(line.begin()+2, line.end(), child_boundary_->begin()));
    }

    bool is_parent_multipart_end(std::string const & line) const
    {
        return (parent_boundary_ && parent_boundary_->size() + 4 == line.size() &&
                line[0] == '-' && line[1] == '-' && line[line.size()-1] == '-' && line[line.size()-2] == '-' &&
                std::equal(line.begin()+2, line.end()-2, parent_boundary_->begin()));
    }

private:
    std::string const * child_boundary_;
    std::string const * parent_boundary_;
    boost::iostreams::filtering_ostream * out_;
    parser_impl<Message> * subpart_ = nullptr;
};


template <typename Message>
struct parser_impl
    : public boost::variant<header_parser, payload_parser<Message> >
{
    using header_type = header_parser;
    using payload_type = payload_parser<Message>;
    using base_type = boost::variant<header_type, payload_type>;
    using streambuf_type = std::basic_streambuf<typename Message::char_type, typename Message::traits_type>;

    struct visitor : boost::static_visitor<bool>
    {
        explicit visitor(std::string const & line, parser_impl & impl)
            : line_(line), impl_(impl) {}

        template <typename Parser>
        bool operator()(Parser & parser) const { return parser(line_, impl_); }

    private:
        std::string const & line_;
        parser_impl & impl_;
    };

    explicit parser_impl(Message & email_, std::string const * boundary = nullptr)
        : base_type(header_type(boundary)), email(email_) {}

    void parse_line(std::string const & line)
    {
        while(!boost::apply_visitor(visitor(line, *this), *this))
            ;
    }

    void to_payload(std::string const * boundary)
    {
        std::string charset;
        std::string const * child_boundary;
        encoding enc;
        bool text_mode;
        bool is_format_flowed;
        bool is_delete_space;
        get_encoding_params(email.headers, charset, child_boundary, enc, text_mode, is_format_flowed, is_delete_space);

        if (text_mode) {
            *static_cast<base_type *>(this) = payload_type(
                child_boundary, boundary, email, enc, charset);
        } else {
            *static_cast<base_type *>(this) = payload_type(
                child_boundary, boundary, email, enc);
        }
    }

    void to_subpart(std::string const * boundary, parser_impl *& subpart)
    {
        if (subpart) {
            delete subpart;
            subpart = nullptr;
        }
        subpart = new parser_impl(email.add_subpart(), boundary);
    }

    void complete()
    {
        // ヘッダーに戻るというより、payload_parser をデストラクタさせる
        *static_cast<base_type *>(this) = header_type(nullptr);
        in_progress = false;
    }

    Message & email;
    bool in_progress = true;
};

}  // detail

template <typename String>
struct basic_email_parser<String>::impl
    : detail::parser_impl< basic_message<String> >
{
    explicit impl(basic_email<String> & email)
        : detail::parser_impl< basic_message<String> >(*email) {}

    std::streamsize write(char const * beg, std::streamsize size)
    {
        if (size == EOF || !this->in_progress)
            return EOF;

        if (size <= 0)
            return size;

        char sep[] = { '\r', '\n' };
        char const * end = beg + size;

        // 前回の最後が '\r' のときで 今回の最初が '\n' のときは '\n' を飛ばす
        if (last_ == '\r' && *beg == '\n')
            ++beg;
        last_ = end[-1];

        for(char const * it; (it = std::find_first_of(beg, end, sep, sep+2)) != end; beg = it) {
            line_.append(beg, it);  // 改行コードは含まない
            this->parse_line(line_);
            line_.clear();
            if (*it == '\r')
                ++it;
            if (it == end)
                return size;
            if (*it == '\n')
                ++it;
            if (it == end)
                return size;
            // 中断
            if (!this->in_progress)
                return size - (end - it);
        }

        // TODO: line_ は RFC に則った許容量を超えたときにエラーにしたほうがいいかも

        line_.append(beg, end);
        return size;
    }

    std::string line_ = {};
    char last_ = '\0';
};

template <typename String>
inline basic_email_parser<String>::basic_email_parser(basic_email<String> & email)
    : impl_(new impl(email)) {}

template <typename String>
inline basic_email_parser<String>::operator bool() const
{
    return !impl_->in_progress;
}

template <typename String>
inline std::streamsize basic_email_parser<String>::write(char_type const * s, std::streamsize n)
{
    return impl_->write(s, n);
}

} }
