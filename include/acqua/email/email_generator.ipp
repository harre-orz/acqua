#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/email/headers.hpp>
#include <acqua/email/message.hpp>
#include <acqua/email/encode_mimeheader.hpp>
#include <acqua/email/detail/encoding.hpp>
#include <acqua/iostreams/ascii_filter.hpp>
#include <acqua/iostreams/qprint_filter.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/iostreams/istream_codecvt.hpp>
#include <boost/variant.hpp>

namespace acqua { namespace email {

namespace detail {

template <typename Message>
struct generator_impl;

template <typename It>
class header_generator
{
public:
    explicit header_generator(It it, std::string const * boundary)
        : it_(it), boundary_(boundary) {}

    template <typename Impl>
    bool operator()(std::ostream & os, Impl & impl)
    {
        if (it_ == impl.email.headers.end()) {
            os << '\r' << '\n';
            impl.to_payload(boundary_);
        } else {
            encode_mimeheader(os, it_->first, it_->second.str(), it_->second);
            os << '\r' << '\n';
            ++it_;
        }
        return true;
    }

private:
    It it_;
    std::string const * boundary_;
};

template <typename Message>
class payload_generator
{
    using char_type = typename Message::char_type;
    using traits_type = typename Message::traits_type;
    using streambuf_type = typename Message::streambuf_type;
    using string_type = typename Message::value_type;
    using message_iterator = typename Message::iterator;

public:
    // 文字コードを変換しない
    explicit payload_generator(std::string const * child_boundary, std::string const * parent_boundary,
                               Message & email, encoding enc)
        : child_boundary_(child_boundary), parent_boundary_(parent_boundary)
        , in_(new boost::iostreams::filtering_istream), it_(email.begin()), email_(&email), text_mode_(false)
    {
        // switch(enc) {
        //     case encoding::ascii:
        //         break;
        //     case encoding::qprint:
        //         in_->push(acqua::iostreams::qprint_encoder());
        //         break;
        //     case encoding::base64:
        //         in_->push(acqua::iostreams::base64_encoder());
        //         break;
        // }
        in_->push(acqua::iostreams::istream_code_converter<char>(static_cast<streambuf_type *>(email)));
    }

    // 文字コードを変換する
    explicit payload_generator(std::string const * child_boundary, std::string const * parent_boundary,
                               Message & email, encoding enc, std::string const & charset)
        : child_boundary_(child_boundary), parent_boundary_(parent_boundary)
        , in_(new boost::iostreams::filtering_istream), it_(email.begin()), email_(&email), text_mode_(true)
    {
        // switch(enc) {
        //     case encoding::ascii:
        //         in_->push(acqua::iostreams::ascii_encoder());
        //         break;
        //     case encoding::qprint:
        //         in_->push(acqua::iostreams::qprint_encoder());
        //         break;
        //     case encoding::base64:
        //         in_->push(acqua::iostreams::base64_encoder());
        //         break;
        // }
        in_->push(acqua::iostreams::istream_code_converter<char>(static_cast<streambuf_type *>(email), charset));
    }

    payload_generator(payload_generator const &) = delete;
    payload_generator(payload_generator && rhs) noexcept
        : child_boundary_(rhs.child_boundary_), parent_boundary_(rhs.parent_boundary_)
        , in_(rhs.in_), subpart_(rhs.subpart_)
        , it_(rhs.it_), email_(rhs.email_), text_mode_(rhs.text_mode_)
    {
        rhs.in_ = nullptr;
        rhs.subpart_ = nullptr;
    }

    payload_generator & operator=(payload_generator const &) = delete;
    payload_generator & operator=(payload_generator && rhs) noexcept
    {
        if (this != &rhs) {
            child_boundary_ = rhs.child_boundary_;
            parent_boundary_ = rhs.parent_boundary_;
            in_ = rhs.in_;
            rhs.in_ = nullptr;
            subpart_ = rhs.subpart_;
            rhs.subpart_ = nullptr;
            email_ = rhs.email_;
            it_ = rhs.it_;
            text_mode_ = rhs.text_mode_;
        }
        return *this;
    }

    ~payload_generator() noexcept
    {
        delete subpart_;
        delete in_;
    }

    template <typename Impl>
    bool operator()(std::ostream & os, Impl & impl)
    {
        if (subpart_ && subpart_->in_progress) {
            subpart_->generate(os);
        } else if (in_->good()) {
            if (text_mode_) {
                std::string line;
                if (!std::getline(*in_, line))
                    return false;
                os << line;
            } else {
                char buf[4096];
                in_->read(buf, sizeof(buf));
                std::streamsize size = in_->gcount();
                if (size <= 0)
                    return false;
                os.write(buf, size);
            }
        } else if (child_boundary_ && it_ != email_->end()) {
            os << '-' << '-' << *child_boundary_ << '\n' << '\n';
            subpart_ = new generator_impl<Message>(*it_++);
        } else {
            if (parent_boundary_) {
                os << '-' << '-' << *parent_boundary_ << '-' << '-' << '\r' << '\n';
            } else {
                os << '\r' << '\n' << '.' << '\r' << '\n';
            }
            impl.complete();
        }

        return true;
    }

private:
    std::string const * child_boundary_;
    std::string const * parent_boundary_;
    boost::iostreams::filtering_istream * in_;
    generator_impl<Message> * subpart_ = nullptr;
    message_iterator it_;
    Message * email_;
    bool text_mode_;
};

template <typename Message>
struct generator_impl
    : boost::variant< header_generator<typename Message::headers_type::const_iterator>, payload_generator<Message> >
{
    using header_type = header_generator<typename Message::headers_type::const_iterator>;
    using payload_type = payload_generator<Message>;
    using base_type = boost::variant<header_type, payload_type>;

    struct visitor : boost::static_visitor<bool>
    {
        explicit visitor(std::ostream & os, generator_impl & impl)
            : os_(os), impl_(impl) {}

        template <typename Generator>
        bool operator()(Generator & generator) const { return generator(os_, impl_); }

    private:
        std::ostream & os_;
        generator_impl & impl_;

    };

public:
    explicit generator_impl(Message & email_)
        : base_type(header_type(email_.headers.begin(), nullptr))
        , email(email_) {}

    void generate(std::ostream & os)
    {
        while(!boost::apply_visitor(visitor(os, *this), *this))
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
                child_boundary, boundary, email,enc);
        }
    }

    void complete()
    {
        in_progress = false;
    }

public:
    Message & email;
    bool in_progress = true;
};

} // detail

template <typename String>
struct basic_email_generator<String>::impl
    : detail::generator_impl< basic_message<String> >
{
    explicit impl(basic_email<String> & email)
        : detail::generator_impl< basic_message<String> >(*email) {}

    std::streamsize read(char * str, std::streamsize size)
    {
        if (!this->in_progress || size <= 0)
            return EOF;

        std::size_t rest = static_cast<std::size_t>(size);
        while(this->in_progress && rest > 0) {
            if (buf_.empty()) {
                boost::iostreams::filtering_ostream out(boost::iostreams::back_inserter(buf_));
                this->generate(out);
            }
            std::size_t len = std::min<std::size_t>(buf_.size(), rest);
            std::memcpy(str, buf_.c_str(), len);
            str += len;
            rest -= len;
            buf_.erase(0, len);
        }

        return (size - static_cast<std::streamsize>(rest));
    }

private:
    std::string buf_;
};

template <typename String>
inline basic_email_generator<String>::basic_email_generator(basic_email<String> & email)
    : impl_(new impl{email}) {}

template <typename String>
inline basic_email_generator<String>::operator bool() const
{
    return !impl_->in_progress;
}

template <typename String>
inline std::streamsize basic_email_generator<String>::read(char_type * s, std::streamsize n)
{
    return impl_->read(s, n);
}

} }
