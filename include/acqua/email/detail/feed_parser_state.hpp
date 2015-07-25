#pragma once

#include <iostream>
#include <locale>
#include <boost/system/error_code.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <acqua/text/mime_header.hpp>
#include <acqua/email/detail/noop.hpp>
#include <acqua/email/detail/ascii.hpp>
#include <acqua/email/detail/qprint.hpp>
#include <acqua/email/detail/base64.hpp>

namespace acqua { namespace email { namespace detail {

class head_parser
{
public:
    head_parser()
        : boundary_(nullptr) {}

    head_parser(std::string const * boundary)
        : boundary_(boundary) {}

    template <typename State>
    bool operator()(std::string const & line, State & state)
    {
        auto it = line.begin();
        if (line.empty()) {
            // ヘッダーの終了
            append_to_header(state);
            state.change_to_body(boundary_);
            return true;
        } else if (line.size() == 1 && line[0] == '.') {
            // メールの終了
            append_to_header(state);
            state.change_to_terminated();
            return false;
        } else if (!std::isspace(*it, std::locale::classic())) {
            // 最初のヘッダー名、もしくは次のヘッダー名
            if ((it = std::find(it, line.end(), ':')) == line.end()) {
                // ヘッダー名と値を分けれない場合は、ヘッダー解析を打ち切り、line を本文として処理する
                state.change_to_body(boundary_);
                return false;
            }
            append_to_header(state);
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
    template <typename State>
    void append_to_header(State & state) const
    {
        if (!name_.empty() && !buffer_.empty()) {
            auto & disp = state.header(name_);
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

template <typename State, typename Decoder>
class body_parser
{
public:
    body_parser(Decoder decode, std::string const * child_boundary, std::string const * parent_boundary)
        : decode_(decode), child_boundary_(child_boundary), parent_boundary_(parent_boundary) {}

    bool operator()(std::string const & line, State & state)
    {
        if (line.size() == 1 && line[0] == '.') {
            decode_.flush(state.body());
            state.change_to_terminated();
            return false;
        } else if (is_child_multipart_begin(line)) {
            state.change_to_subpart(subpart_, child_boundary_);
        } else if (is_parent_multipart_end(line)) {
            decode_.flush(state.body());
            state.change_to_terminated();
        } else if (subpart_ && !subpart_->is_terminated()) {
            subpart_->do_parse_oneline(line);
        } else {
            decode_.write(state.body(), line);
        }
        return true;
    }

    void do_flush(State & state)
    {
        decode_.flush(state.body());
    }

private:
    bool is_child_multipart_begin(std::string const & line) const
    {
        return (child_boundary_ && child_boundary_->size() == line.size() + 2 &&
                line[0] == '-' && line[1] == '-' &&
                std::equal(line.begin()+2, line.end(), child_boundary_->begin()));
    }

    bool is_parent_multipart_end(std::string const & line) const
    {
        return (parent_boundary_ && parent_boundary_->size() == line.size() + 4 &&
                line[0] == '-' && line[1] == '-' && line[2] == '-' && line[3] == '-' &&
                std::equal(line.begin()+2, line.end()-2, child_boundary_->begin()));
    }

private:
    Decoder decode_;
    std::string const * child_boundary_;
    std::string const * parent_boundary_;
    std::unique_ptr<State> subpart_;
};


template <typename Message, typename Literals>
class feed_parser_state
    : public boost::variant< head_parser,
                             body_parser<feed_parser_state<Message, Literals>, noop_decoder>,
                             body_parser<feed_parser_state<Message, Literals>, ascii_decoder>,
                             body_parser<feed_parser_state<Message, Literals>, qprint_decoder>,
                             body_parser<feed_parser_state<Message, Literals>, base64_text_decoder>,
                             body_parser<feed_parser_state<Message, Literals>, base64_binary_decoder>
                             >
    , Literals
{
    using literals_type = Literals;
    using noop_binary_parser = body_parser<feed_parser_state<Message, Literals>, noop_decoder>;
    using ascii_text_parser = body_parser<feed_parser_state<Message, Literals>,ascii_decoder>;
    using qprint_text_parser = body_parser<feed_parser_state<Message, Literals>, qprint_decoder>;
    using base64_text_parser = body_parser<feed_parser_state<Message, Literals>, base64_text_decoder>;
    using base64_binary_parser = body_parser<feed_parser_state<Message, Literals>, base64_binary_decoder>;
    using base_type = boost::variant< head_parser,
                                      noop_binary_parser,
                                      ascii_text_parser,
                                      qprint_text_parser,
                                      base64_text_parser,
                                      base64_binary_parser >;

    struct feed_visitor : boost::static_visitor<>
    {
        std::string const & line_;
        feed_parser_state & state_;

        feed_visitor(std::string const & line, feed_parser_state & state)
            : line_(line), state_(state) {}

        template <typename Parser>
        void operator()(Parser & parser) const
        {
            while(!state_.is_terminated() && !parser(line_, state_))
                ;
        }
    };

    struct flush_visitor : boost::static_visitor<>
    {
        feed_parser_state & state_;
        explicit flush_visitor(feed_parser_state & state)
            : state_(state) {}

        void operator()(head_parser &) const
        {
        }

        template <typename Decoder>
        void operator()(body_parser<feed_parser_state, Decoder> & parser) const
        {
            parser.do_flush(state_);
        }
    };

public:
    using message_type = Message;
    using ostream_type = std::basic_ostream<typename message_type::char_type>;

public:
    feed_parser_state(boost::system::error_code & error, message_type & message)
        : error_(error), message_(message), body_(message) {}

    feed_parser_state(feed_parser_state & state, std::string const * boundary)
        : base_type(head_parser(boundary))
        , error_(state.error_), message_(state.message_.add_subpart()), body_(message_) {}

    /*!
      解析が終了したか？.
     */
    bool is_terminated() const
    {
        return is_terminated_;
    }

    /*!
      パーサの状態を、解析完了に遷移します.
     */
    void change_to_terminated()
    {
        is_terminated_ = true;
    }

    /*!
      パーサの状態を、本文解析に遷移します.
    */
    void change_to_body(std::string const * boundary)
    {
        std::string const * child_boundary = nullptr;
        std::string charset = "us-ascii";
        bool text_mode = false;  // 改行コードを変換するか？
        bool is_format_flowed = false;
        bool is_delete_space = false;

        auto & next_parser = *static_cast<base_type *>(this);
        auto it = message_.find(literals_type::content_type);
        if (it != message_.end()) {
            // バウンダリを探す
            auto it2 = it->second.find(literals_type::boundary);
            if (it2 != it->second.end())
                child_boundary = &it2->second;

            // Content-Type のメジャータイプが text のときは、改行コードを自動変換する
            if ((text_mode = boost::algorithm::istarts_with(it->second.str(), literals_type::text_))) {
                it2 = it->second.find(literals_type::charset);
                if (it2 != it->second.end())
                    charset = it2->second;
                // RFC3676 対応
                it2 = it->second.find(literals_type::format);
                is_format_flowed = boost::algorithm::iequals(it2->second, literals_type::flowed);
                it2 = it->second.find(literals_type::delsp);
                is_delete_space = boost::algorithm::iequals(it2->second, literals_type::yes);
            }
        }

        it = message_.find(literals_type::content_transfer_encoding);
        if (it != message_.end()) {
            if (boost::algorithm::iequals(it->second.str(), literals_type::base64)) {
                if (text_mode) {
                    next_parser = base64_text_parser(base64_text_decoder(charset), child_boundary, boundary);
                } else {
                    next_parser = base64_binary_parser(base64_binary_decoder(), child_boundary, boundary);
                }
                return;
            }
            if (boost::algorithm::iequals(it->second.str(), literals_type::quoted_printable)) {
                // text-mode しかない
                next_parser = qprint_text_parser(qprint_decoder(charset), child_boundary, boundary);
                return;
            }
        }

        if (text_mode) {
            next_parser = ascii_text_parser(ascii_decoder(charset, is_format_flowed, is_delete_space), child_boundary, boundary);
        } else {
            next_parser = noop_binary_parser(noop_decoder(), child_boundary, boundary);
        }
    }

    /*!
      パーサの状態を、本文の子パートに遷移します.
     */
    void change_to_subpart(std::unique_ptr<feed_parser_state> & subpart, std::string const * boundary)
    {
        if (subpart) boost::apply_visitor(flush_visitor(*subpart), *subpart);
        subpart.reset(new feed_parser_state(*this, boundary));
    }

    /*!
      一行の文字列（改行コードが覗かれた）を解析して結果を message_ に格納します.
     */
    void do_parse_oneline(std::string const & line)
    {
        boost::apply_visitor(feed_visitor(line, *this), *this);
    }

    template <typename Key>
    typename message_type::disposition & header(Key const & key)
    {
        return message_[key];
    }

    ostream_type & body()
    {
        return body_;
    }

private:
    boost::system::error_code & error_;
    message_type & message_;
    ostream_type body_;
    bool is_terminated_ = false;
};

} } }
