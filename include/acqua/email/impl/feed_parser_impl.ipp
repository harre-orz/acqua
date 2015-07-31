/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/algorithm/string.hpp>
#include <acqua/text/mime_header.hpp>
#include <acqua/email/detail/noop_decoder.hpp>
#include <acqua/email/detail/ascii_decoder.hpp>
#include <acqua/email/detail/qprint_decoder.hpp>
#include <acqua/email/detail/base64.hpp>

namespace acqua { namespace email { namespace detail {

class head_parser
{
public:
    head_parser()
        : boundary_(nullptr) {}

    head_parser(std::string const * boundary)
        : boundary_(boundary) {}

    template <typename Impl>
    bool operator()(std::string const & line, Impl & impl)
    {
        auto it = line.begin();
        if (line.empty()) {
            // ヘッダーの終了
            append_to_header(impl);
            impl.to_body(boundary_);
            return true;
        } else if (line.size() == 1 && line[0] == '.') {
            // メールの終了
            append_to_header(impl);
            impl.to_terminated();
            return false;
        } else if (!std::isspace(*it, std::locale::classic())) {
            // 最初のヘッダー名、もしくは次のヘッダー名
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダー名と値を分けれない場合は、ヘッダー解析を打ち切り、line を本文として処理する
                impl.to_body(boundary_);
                return false;
            }
            append_to_header(impl);
            name_.assign(line.begin(), it);
            buffer_.clear();
        }
        while(std::isspace(*++it, std::locale::classic()))
            ;
        if (!buffer_.empty())
            buffer_.push_back(' ');
        buffer_.append(it, line.end());
        return true;
    }

private:
    template <typename Impl>
    void append_to_header(Impl & impl) const
    {
        if (!name_.empty() && !buffer_.empty()) {
            auto & disp = impl.header(name_);
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


template <typename Impl, typename Decoder>
class body_parser
{
public:
    body_parser(Decoder decode, std::string const * child_boundary, std::string const * parent_boundary)
        : decode_(decode), child_boundary_(child_boundary), parent_boundary_(parent_boundary) {}

    bool operator()(std::string const & line, Impl & impl)
    {
        if (line.size() == 1 && line[0] == '.') {
            decode_.flush(impl.body());
            impl.to_terminated();
            return false;
        } else if (is_child_multipart_begin(line)) {
            impl.to_subpart(subpart_, child_boundary_);
        } else if (is_parent_multipart_end(line)) {
            decode_.flush(impl.body());
            impl.to_terminated();
        } else if (subpart_ && !subpart_->is_terminated()) {
            subpart_->do_parse_line(line);
        } else {
            decode_.write(impl.body(), line);
        }
        return true;
    }

    void flush(Impl & impl)
    {
        decode_.flush(impl.body());
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
    Decoder decode_;
    std::string const * child_boundary_;
    std::string const * parent_boundary_;
    std::unique_ptr<Impl> subpart_;
};

}

using namespace acqua::email::detail;

template <typename Mail>
class feed_parser<Mail>::impl
    : public boost::variant< head_parser,
                             body_parser<impl, noop_decoder>,
                             body_parser<impl, ascii_decoder>,
                             body_parser<impl, qprint_decoder>,
                             body_parser<impl, base64_text_decoder>,
                             body_parser<impl, base64_binary_decoder>
                             >
{
private:
    struct feed_visitor : boost::static_visitor<bool>
    {
        std::string const & line_;
        impl & impl_;
        feed_visitor(std::string const & line, impl & impl)
            : line_(line), impl_(impl) {}

        template <typename Parser>
        bool operator()(Parser & parser) const
        {
            return parser(line_, impl_);
        }
    };

    struct flush_visitor : boost::static_visitor<>
    {
        impl & impl_;
        flush_visitor(impl & impl)
            : impl_(impl) {}

        void operator()(head_parser &) const
        {
        }

        template <typename Decoder>
        void operator()(body_parser<impl, Decoder> & parser) const
        {
            parser.flush(impl_);
        }
    };

    using message_type = basic_message<typename Mail::value_type>;
    using ostream_type = std::basic_ostream<typename Mail::char_type, typename Mail::traits_type>;

public:
    using noop = body_parser<impl, noop_decoder>;
    using ascii = body_parser<impl, ascii_decoder>;
    using qprint = body_parser<impl, qprint_decoder>;
    using base64txt = body_parser<impl, base64_text_decoder>;
    using base64bin = body_parser<impl, base64_binary_decoder>;
    using base_type = boost::variant< head_parser, noop, ascii, qprint, base64txt, base64bin >;

    impl(boost::system::error_code & error, message_type & mail, std::string const * boundary = nullptr)
        : base_type(head_parser(boundary)), error_(error), mail_(mail), os_(mail_) {}

    /*!
      解析が終了したか？.
     */
    bool is_terminated() const
    {
        return is_terminated_;
    }

    /*!
      一行の文字列（改行コードが覗かれた）を解析して結果を message_ に格納する.
     */
    void do_parse_line(std::string const & line)
    {
        while(!is_terminated() && !boost::apply_visitor(feed_visitor(line, *this), *this))
            ;
    }

    template <typename Key>
    typename Mail::disposition & header(Key const & key)
    {
        return mail_[key];
    }

    ostream_type & body()
    {
        return os_;
    }

    /*!
      パーサの状態を、本文解析に遷移する.
    */
    void to_body(std::string const * boundary)
    {
        std::string const * child_boundary = nullptr;
        std::string charset = "us-ascii";
        bool text_mode = false;  // 改行コードを変換するか？
        bool is_format_flowed = false;
        bool is_delete_space = false;

        auto & next = *static_cast<base_type *>(this);
        auto it = mail_.find("Content-Type");
        if (it != mail_.end()) {
            // バウンダリを探す
            auto it2 = it->second.find("boundary");
            if (it2 != it->second.end())
                child_boundary = &it2->second;

            // Content-Type のメジャータイプが text のときは、改行コードを自動変換する
            if ((text_mode = boost::algorithm::istarts_with(it->second.str(), "text/"))) {
                it2 = it->second.find("charset");
                if (it2 != it->second.end())
                    charset = it2->second;
                // RFC3676 対応
                it2 = it->second.find("format");
                if (it2 != it->second.end())
                    is_format_flowed = boost::algorithm::iequals(it2->second, "flowed");
                it2 = it->second.find("delsp");
                if (it2 != it->second.end())
                    is_delete_space = boost::algorithm::iequals(it2->second, "yes");
            }
        }

        it = mail_.find("Content-Transfer-Encoding");
        if (it != mail_.end()) {
            if (boost::algorithm::iequals(it->second.str(), "base64")) {
                if (text_mode) {
                    std::cout << "base64txt " << charset << std::endl;
                    next = base64txt(base64_text_decoder(charset), child_boundary, boundary);
                } else {
                    std::cout << "base64bin " << std::endl;
                    next = base64bin(base64_binary_decoder(), child_boundary, boundary);
                }
                return;
            }
            if (boost::algorithm::iequals(it->second.str(), "quoted-printable")) {
                // text-mode しかない
                next = qprint(qprint_decoder(charset), child_boundary, boundary);
                return;
            }
        }

        if (text_mode) {
            next = ascii(ascii_decoder(charset, is_format_flowed, is_delete_space), child_boundary, boundary);
        } else {
            next = noop(noop_decoder(), child_boundary, boundary);
        }
    }

    /*!
      パーサの状態を、本文の子パートに遷移する.
     */
    void to_subpart(std::unique_ptr<impl> & subpart, std::string const * boundary)
    {
        if (subpart) boost::apply_visitor(flush_visitor(*subpart), *subpart);
        subpart.reset(new impl(error_, mail_.add_subpart(), boundary));
    }

    void to_terminated()
    {
        is_terminated_ = true;
    }

private:
    boost::system::error_code & error_;
    message_type & mail_;
    ostream_type os_;
    bool is_terminated_ = false;
};

} }
