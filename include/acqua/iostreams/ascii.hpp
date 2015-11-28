#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <acqua/iostreams/newline.hpp>
#include <acqua/iostreams/detail/newline_base.hpp>

namespace acqua { namespace iostreams {

/*!
  メールの 7bit foramt_flowed 指定におけるエンコードを行うクラス.
 */
class ascii_encoder
    : public detail::newline_base< ascii_encoder >
{
private:
    using base_type = ascii_encoder::base_type;

public:
    using char_type = char;
    using category = boost::iostreams::output_filter_tag;

public:
    explicit ascii_encoder(newline nl = newline::crln, std::size_t size = 77)
        : base_type(nl, size) {}

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        return (ch == '\r') ? true
            :  (ch == '\n') ? base_type::put_ln(sink)
            :  base_type::put(sink, ch);
    }

    // overrided by base_type
    template <typename Sink>
    bool line_break(Sink & sink)
    {
        return boost::iostreams::put(sink, ' ') && base_type::put_ln(sink);
    }
};

/*!
  メールの 7bit foramt_flowed 指定におけるデコードを行うクラス.
 */
class ascii_decoder
{
public:
    using char_type = char;
    using category = boost::iostreams::input_filter_tag;

public:
    template <typename Source>
    int get(Source & src)
    {
        int ch;
        if (prior_) {
            ch = prior_;
            prior_ = 0;
        } else {
            ch = boost::iostreams::get(src);
        }

        if (ch == ' ' || ch == '\t') {
            prior_ = ch;
            ch = boost::iostreams::get(src);
            if (ch == '\r') { prior_ = 0; ch = boost::iostreams::get(src); }
            if (ch == '\n') { prior_ = 0; ch = boost::iostreams::get(src); }
            if (prior_) std::swap(ch, prior_);
        } else if(ch == '\r') {
            ch = boost::iostreams::get(src);
            if (ch != '\n') {
                prior_ = ch;
                ch = '\n';
            }
        }

        return ch;
    }

private:
    int prior_ = 0;
};

} }
