/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <sstream>
#include <boost/locale/encoding.hpp>
#include <acqua/text/linefeed.hpp>
#include <acqua/text/detail/qprint_impl.hpp>

namespace acqua { namespace text {

template <typename CharT>
class qprint_encoder
{
public:
    explicit qprint_encoder(std::string const & charset = "UTF-8", linefeed_type linefeed = linefeed_type::crln, std::size_t indent = 0, std::size_t width = 80)
        : impl_(width, indent), charset_(charset), linefeed_(linefeed) {}

    void push(std::basic_string<CharT> const & str)
    {
        if (str.empty())
            return;

        std::string buf = boost::locale::conv::from_utf(str, charset_);
        std::size_t i = 0, n = buf.size();

        do {
            std::streamsize rest = impl_.write(oss_, buf.c_str() + i, buf.size() - i);
            if (rest < 0)
                return;
            i += rest;
            if (i >= n)
                return;
            oss_ << linefeed_;
        } while(true);
    }

    std::string str()
    {
        impl_.flush(oss_);
        return oss_.str();
    }

private:
    detail::qprint_encoder_impl impl_;
    std::string charset_;
    linefeed_type linefeed_;
    std::ostringstream oss_;
};


template <typename CharT>
class qprint_decoder
{
public:
    explicit qprint_decoder(std::string const & charset = "UTF-8")
        : charset_(charset) {}

    void push(std::string const & str)
    {
        std::streamsize rest;
        for(std::size_t i = 0; i < str.size(); i += rest) {
            rest = impl_.write(oss_, str.c_str() + i, str.size() - i);
            if (rest < 0)
                return;
        }
    }

    std::basic_string<CharT> str()
    {
        impl_.flush(oss_);
        return boost::locale::conv::to_utf<CharT>(oss_.str(), charset_);
    }

private:
    detail::qprint_decoder_impl impl_;
    std::string charset_;
    std::ostringstream oss_;
};

} }
