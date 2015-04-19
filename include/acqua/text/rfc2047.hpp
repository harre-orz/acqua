#pragma once

#include <sstream>
#include <boost/locale/encoding.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/text/linefeed.hpp>
#include <acqua/text/detail/base64_impl.hpp>
#include <acqua/text/detail/qprint_impl.hpp>

namespace acqua { namespace text {

template <typename CharT, typename Impl = detail::base64_encoder_impl>
class rfc2047_encoder
{
public:
    explicit rfc2047_encoder(std::string const & charset = "US-ASCII", linefeed_type linefeed = linefeed_type::crln,
                             std::size_t indent = 0, std::size_t width = 80)
        : impl_(width, indent), charset_(charset), linefeed_(linefeed) {}

    void push(std::basic_string<CharT> const & str)
    {
        std::string buffer = boost::locale::conv::from_utf(str, charset_);
        std::size_t i = 0;
        do {
            if (is_newline_) {
                oss_ << ' ' << '=' << '?' << charset_ << '?' << encoder_symbol(impl_) << '?';
                is_newline_ = false;
            }
            std::streamsize rest = impl_.write(oss_, buffer.c_str() + i, buffer.size() - i);
            if (rest < 0)
                return;
            i += rest;
            if (i >= buffer.size())
                break;
            oss_ << '?' << '=' << linefeed_;
            is_newline_ = true;
        } while(true);
    }

    std::string str()
    {
        impl_.flush(oss_);
        if (!is_newline_) {
            oss_ << '?' << '=';
            is_newline_ = true;
        }
        return oss_.str();
    }

private:
    char encoder_symbol(detail::base64_encoder_impl const &) const
    {
        return 'B';
    }

    char encoder_symbol(detail::qprint_encoder_impl const &) const
    {
        return 'Q';
    }

private:
    Impl impl_;
    std::string charset_;
    linefeed_type linefeed_;
    std::ostringstream oss_;
    bool is_newline_ = true;
};

template <typename CharT>
class rfc2047_decoder
{
public:
    explicit rfc2047_decoder(std::string const & charset = "US-ASCII")
        : charset_(charset) {}

    void push(std::string const & str)
    {
        namespace qi = boost::spirit::qi;

        auto it = str.begin();
        while(it != str.end()) {
            std::string charset, buffer;
            char sym;
            if (qi::parse(it, str.end(),
                          "=?" >> +(qi::char_ - '?') >> '?' >> qi::char_("BbQq") >> '?' >> +(qi::char_ - '?') >> "?=",
                          charset, sym, buffer)) {
                charset_ = charset;
                switch(sym) {
                    case 'Q': case 'q': {
                        detail::qprint_decoder_impl impl;
                        impl.write(oss_, buffer.c_str(), buffer.size());
                        impl.flush(oss_);
                        break;
                    }
                    case 'B': case 'b':
                    default: {
                        detail::base64_decoder_impl impl;
                        impl.write(oss_, buffer.c_str(), buffer.size());
                        impl.flush(oss_);
                        break;
                    }
                }
            } else {
                while(it != str.end()) {
                    if (std::isspace(*it))
                        break;
                    oss_ << *it++;
                }
            }

            while(it != str.end() && std::isspace(*it))
                ++it;
        }
    }

    std::basic_string<CharT> str()
    {
        return boost::locale::conv::to_utf<CharT>(oss_.str(), charset_);
    }

private:
    std::string charset_;
    std::ostringstream oss_;
};

} }
