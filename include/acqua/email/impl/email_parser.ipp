#pragma once

#include <iostream>
#include <algorithm>
#include <boost/variant.hpp>
#include <acqua/email/email_parser.hpp>
#include <acqua/email/message.hpp>

namespace acqua { namespace email { namespace detail {

enum class encoding { ascii, qprint, base64 };

class header_parser
{
public:
    header_parser()
        : boundary_(nullptr) {}

    header_parser(std::string const * boundary)
        : boundary_(boundary) {}

    template <typename Impl>
    bool operator()(std::string const & line, Impl & impl)
    {
        if (line.empty()) {
            // ヘッダー終了
            append(impl.email);
            impl.new_payload(boundary_);
            return true;
        }

        if (line.size() == 1 && line[0] == '.') {
            // メールの終了記号
            append(impl.email);
            impl.failured();
            return true;
        }

        auto it = line.begin();
        if (!std::isspace(*it, std::locale::classic())) {
            // 新しいヘッダー
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダー名と値を分けられないときは、ヘッダーの解析を打ち切り line を本文として処理させる
                impl.new_payload(boundary_);
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
                // TODO: disposition のパース
                disp = params_;
            } else {
                // TODO: disposition のパース
                disp = params_;
            }
        }
    }

private:
    std::string const * boundary_;
    std::string name_;
    std::string params_;
};


template <typename Impl>
struct text_payload_parser
{
    explicit text_payload_parser(std::string const * child_boundary, std::string const * parent_boundary,
                                 encoding enc, std::string const & charset)
    {
    }

    bool operator()(std::string const & line, Impl & impl)
    {
    }

    void flush(Impl & impl)
    {
    }
};


template <typename Impl>
struct binary_payload_parser
{
    explicit binary_payload_parser(std::string const * child_boundary, std::string const * parent_boundary,
                                   encoding enc)
    {
    }

    bool operator()(std::string const & line, Impl & impl)
    {
    }

    void flush(Impl & impl)
    {
    }
};


template <typename Message>
struct parser_impl
    : public boost::variant<
      detail::header_parser,
      detail::text_payload_parser< parser_impl<Message> >,
      detail::binary_payload_parser< parser_impl<Message> >
    >
{
    using header_type = detail::header_parser;
    using text_payload_type = detail::text_payload_parser< parser_impl<Message> >;
    using binary_payload_type = detail::binary_payload_parser< parser_impl<Message> >;
    using base_type = boost::variant<header_type, text_payload_type, binary_payload_type>;

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

    explicit parser_impl(Message & email_, boost::system::error_code & error_)
        : email(email_), error(error_) {}

    void parse_line(std::string const & line)
    {
        while(!boost::apply_visitor(visitor(line, *this), *this))
            ;
    }

    void new_payload(std::string const * boundary)
    {
        std::string const * child_boundary = nullptr;
        std::string charset = "us-ascii";
        bool text_mode = false; // 改行コードの変換を行うか？
        bool is_format_flowed = false;
        bool is_delete_space = false;

        auto & headers = email.headers;
        auto it = headers.find("Content-Type");
        if (it != headers.end()) {
            auto & contenttype = it->second;
            // バウンダリを探す
            auto it2 = contenttype.find("boundary");
            if (it2 != contenttype.end())
                child_boundary = &it2->second;

            // Content-Type による改行コードの自動変換
            if ((text_mode = boost::algorithm::istarts_with(contenttype.str(), "text/"))) {
                it2 = contenttype.find("charset");
                if (it2 != contenttype.end())
                    charset = it2->second;

                // RFC3676 対応
                it2 = contenttype.find("format");
                if (it2 != contenttype.end())
                    is_format_flowed = boost::algorithm::iequals(it2->second, "flowed");
                it2 = contenttype.find("delsp");
                if (it2 != contenttype.end())
                    is_delete_space = boost::algorithm::iequals(it2->second, "yes");
            }
        }

        encoding enc = encoding::ascii;
        it = headers.find("Content-Transfer-Encoding");
        if (it != headers.end()) {
            auto & content_transfer_encoding = it->second;
            if (boost::algorithm::iequals(content_transfer_encoding.str(), "quoted-printable")) {
                enc = encoding::qprint;
                text_mode = true; // バイナリモードは存在しない
            } else if (boost::algorithm::iequals(content_transfer_encoding.str(), "base64")) {
                enc = encoding::base64;
            }
        }

        if (text_mode)
            *static_cast<base_type *>(this) = text_payload_type(child_boundary, boundary, enc, charset);
        else
            *static_cast<base_type *>(this) = binary_payload_type(child_boundary, boundary, enc);
    }

    void failured()
    {
        // TODO: エラーコード
        error = make_error_code(boost::system::errc::bad_address);
        in_progress = false;
    }

    Message & email;
    boost::system::error_code & error;
    bool in_progress = true;
};

}  // detail

template <typename String>
struct basic_email_parser<String>::impl
    : detail::parser_impl< basic_message<String> >
{
    explicit impl(basic_email<String> & email, boost::system::error_code & error)
        : detail::parser_impl< basic_message<String> >(*email, error) {}

    std::streamsize write(char const * beg, std::streamsize size)
    {
        if (size == EOF || !this->in_progress)
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
        }

        // TODO: line_ は RFC に則った許容量を超えたときにエラーにしたほうがいいかも

        // 残り
        line_.append(beg, end);
        return size;
    }

    std::string line_ = {};
    char last_ = '\0';
};

template <typename String>
inline basic_email_parser<String>::basic_email_parser(basic_email<String> & email)
    : impl_(new impl(email, error_)) {}

template <typename String>
inline std::streamsize basic_email_parser<String>::write(char_type const * s, std::streamsize n)
{
    return impl_->write(s, n);
}

} }
