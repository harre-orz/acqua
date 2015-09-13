/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/algorithm/string.hpp>
#include <acqua/email/utils/mime_header.hpp>
#include <acqua/email/utils/noop_decoder.hpp>
#include <acqua/email/utils/ascii_decoder.hpp>
#include <acqua/email/utils/qprint_decoder.hpp>
#include <acqua/email/utils/base64_decoder.hpp>

namespace acqua { namespace email { namespace detail {

using namespace acqua::email::utils;

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
        auto it = line.begin();
        if (line.empty()) {
            // ヘッダーの終了
            append_to_header(impl);
            impl.to_payload(boundary_);
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
                impl.to_payload(boundary_);
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
                mime_header::decode(buffer_.begin(), buffer_.end(), disp.str(), disp);
            } else {
                mime_header::decode(buffer_.begin(), buffer_.end(), disp.str());
            }
        }
    }

private:
    std::string const * boundary_;
    std::string name_;
    std::string buffer_;
};


template <typename Impl, typename Decoder>
class payload_parser
    : private Decoder
{
public:
    template <typename ... Args>
    payload_parser(std::string const * child_boundary, std::string const * parent_boundary, Args... args)
        : Decoder(args...), child_boundary_(child_boundary), parent_boundary_(parent_boundary) {}

    bool operator()(std::string const & line, Impl & impl)
    {
        if (line.size() == 1 && line[0] == '.') {
            Decoder::flush(impl.payload());
            impl.to_terminated();
            return false;
        } else if (is_child_multipart_begin(line)) {
            impl.to_subpart(subpart_, child_boundary_);
        } else if (is_parent_multipart_end(line)) {
            Decoder::flush(impl.payload());
            impl.to_terminated();
        } else if (subpart_ && !subpart_->is_terminated()) {
            subpart_->do_parse_line(line);
        } else {
            Decoder::write(impl.payload(), line);
        }
        return true;
    }

    void flush(Impl & impl)
    {
        Decoder::flush(impl.payload());
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
    std::unique_ptr<Impl> subpart_;
};


template <typename Mail>
class feed_parser<Mail>::impl : public boost::variant<
    header_parser,
    payload_parser<impl, basic_noop_decoder<char_type> >,
    payload_parser<impl, basic_ascii_decoder<char_type> >,
    payload_parser<impl, basic_qprint_decoder<char_type> >,
    payload_parser<impl, basic_base64_decoder<char_type> >,
    payload_parser<impl, basic_base64_raw_decoder<char_type> >

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

        void operator()(header_parser &) const
        {
        }

        template <typename Decoder>
        void operator()(payload_parser<impl, Decoder> & parser) const
        {
            parser.flush(impl_);
        }
    };

    using ostream_type = std::basic_ostream<char_type, traits_type>;
    using head = header_parser;
    using noop = payload_parser<impl, basic_noop_decoder<char_type> >;
    using ascii = payload_parser<impl, basic_ascii_decoder<char_type> >;
    using qprint = payload_parser<impl, basic_qprint_decoder<char_type> >;
    using base64txt = payload_parser<impl, basic_base64_decoder<char_type> >;
    using base64bin = payload_parser<impl, basic_base64_raw_decoder<char_type> >;
    using base_type = boost::variant<header_parser, noop, ascii, qprint, base64txt, base64bin>;

public:
    explicit impl(boost::system::error_code & error, Mail & mail, std::string const * boundary = nullptr)
        : base_type(head(boundary)), error_(error), mail_(mail), os_(mail_) {}

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

    ostream_type & payload()
    {
        return os_;
    }

    /*!
      パーサの状態を、本文解析に遷移する.
    */
    void to_payload(std::string const * boundary)
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
                if (text_mode) next = base64txt(child_boundary, boundary, charset);
                else           next = base64bin(child_boundary, boundary);
                return;
            }
            if (boost::algorithm::iequals(it->second.str(), "quoted-printable")) {
                // text-mode のみ
                next = qprint(child_boundary, boundary, charset);
                return;
            }
        }

        if (text_mode) next = ascii(child_boundary, boundary, charset, is_format_flowed, is_delete_space);
        else           next = noop(child_boundary, boundary);
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
    Mail & mail_;
    ostream_type os_;
    bool is_terminated_ = false;
};

} } }
