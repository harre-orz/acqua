/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>

#include <acqua/email/utils/noop_encoder.hpp>
#include <acqua/email/utils/ascii_encoder.hpp>
#include <acqua/email/utils/qprint_encoder.hpp>
#include <acqua/email/utils/base64_encoder.hpp>
#include <acqua/email/utils/mime_header.hpp>

namespace acqua { namespace email { namespace detail {

using namespace acqua::email::utils;

template <typename It>
class header_generator
{
public:
    header_generator(It beg, It end)
        : it_(beg), end_(end) {}

    template <typename Impl>
    bool operator()(std::string & line, Impl & impl)
    {
        if (it_ == end_) {
            line += '\r';
            line += '\n';
            impl.to_payload();
        } else {
            mime_header::encode(line, it_->first, it_->second.str(), it_->second);
            line += '\r';
            line += '\n';
            ++it_;
        }

        return true;
    }

private:
    It it_;
    It end_;
};


template <typename Encoder>
class payload_generator
    : private Encoder
{
public:
    template <typename ... Args>
    payload_generator(Args ... args)
        : Encoder(args...)
    {
    }

    template <typename Impl>
    bool operator()(std::string & line, Impl & impl)
    {
        Encoder::read(impl.payload(), line);
        if (line.empty()) {
            line = ".\r\n";
            impl.to_terminated();
        } else if (line == ".\r\n") {
            line = "..\r\n";
        }
        return true;
    }
};


template <typename Mail>
class feed_generator<Mail>::impl : public boost::variant<
    header_generator<const_iterator>,
    payload_generator<basic_noop_encoder<char_type> >,
    payload_generator<basic_ascii_encoder<char_type> >,
    payload_generator<basic_qprint_encoder<char_type> >,
    payload_generator<basic_base64_encoder<char_type> >,
    payload_generator<basic_base64_raw_encoder<char_type> >
    >
{
    using istream_type = std::basic_istream<char>;
    using head = header_generator<typename Mail::const_iterator>;
    using noop = payload_generator<basic_noop_encoder<char_type> >;
    using ascii = payload_generator<basic_ascii_encoder<char_type> >;
    using qprint = payload_generator<basic_qprint_encoder<char_type> >;
    using base64txt = payload_generator<basic_base64_encoder<char_type> >;
    using base64bin = payload_generator<basic_base64_raw_encoder<char_type> >;
    using base_type = boost::variant<head, noop, ascii, qprint, base64txt, base64bin>;

    struct feed_visitor : boost::static_visitor<bool>
    {
        std::string & line_;
        impl & impl_;
        feed_visitor(std::string & line, impl & impl)
            : line_(line), impl_(impl) {}

        template <typename Gen>
        bool operator()(Gen & gen) const
        {
            return gen(line_, impl_);
        }
    };

public:
    explicit impl(Mail & mail)
        : base_type(head(mail.begin(), mail.end()))
        , mail_(mail)
        , is_(mail_)
    {
    }

    bool is_terminated() const
    {
        return is_terminated_;
    }

    bool do_generate_line(std::string & line)
    {
        return boost::apply_visitor(feed_visitor(line, *this), *this);
    }

    void to_terminated()
    {
        is_terminated_ = true;
    }

    void to_payload()
    {
        std::string charset = "us-ascii";
        bool text_mode = false;  // 改行コード変換をするか？
        bool is_format_flowed = false;
        bool is_delete_space = false;

        // is の位置を初期化
        is_.seekg(0, std::ios_base::beg);

        auto & next = *static_cast<base_type *>(this);
        auto it = mail_.find("Content-Type");
        if (it != mail_.end()) {
            // Content-Type のメジャータイプが text のときは、改行コードを自動変換する
            if ((text_mode = boost::algorithm::istarts_with(it->second.str(), "text/"))) {
                auto it2 = it->second.find("charset");
                if (it2 != it->second.end()) {
                    charset = it2->second;
                }
                //RFC3676 対応
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
                if (text_mode) next = base64txt(charset);
                else           next = base64bin();
                return;
            }
            if (boost::algorithm::iequals(it->second.str(), "quoted-printable")) {
                // text-mode のみ
                next = qprint(charset);
                return;
            }
        }

        if (text_mode) next = ascii(charset, is_format_flowed, is_delete_space);
        else           next = noop();
    }

    istream_type & payload()
    {
        return is_;
    }

private:
    Mail & mail_;
    bool is_terminated_ = false;
    istream_type is_;
};

} } }
