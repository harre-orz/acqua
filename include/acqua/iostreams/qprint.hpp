#pragma once

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/scope_exit.hpp>
#include <acqua/iostreams/detail/newline_base.hpp>

namespace acqua { namespace iostreams {

class qprint_encoder
    : public detail::newline_base< qprint_encoder >
{
private:
    using base_type = qprint_encoder::base_type;

public:
    struct category : boost::iostreams::output_filter_tag, boost::iostreams::closable_tag {};
    using char_type = char;

    explicit qprint_encoder(newline nl = newline::none, std::size_t size = std::numeric_limits<std::size_t>::max())
        : base_type(nl, size) {}

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        if (prior_) {
            BOOST_SCOPE_EXIT_ALL(this) { prior_ = 0; };
            if (!((ch == '\r' || ch == '\n') ? escape(sink, prior_) : base_type::put(sink, prior_)))
                return false;
        }

        if (ch == '\r') {
            return true;
        } else if (ch == '\n') {
            return boost::iostreams::put(sink, ch);
        } else if (ch == ' ' || ch == '\t') {
            prior_ = ch;
            return true;
        } else if (ch == '=' || ch < 33 || 126 < ch) {
            return escape(sink, ch);
        } else {
            return base_type::put(sink, ch);
        }
    }

    template <typename Sink>
    void close(Sink & sink)
    {
        if (prior_) {
            boost::iostreams::put(sink, prior_);
            prior_ = 0;
        }
    }

    // overrided by base_type
    template <typename Sink>
    bool line_break(Sink & sink)
    {
        return boost::iostreams::put(sink, '=')
            && base_type::line_break(sink);
    }

private:
    template <typename Sink>
    bool escape(Sink & sink, char hex)
    {
        char upper = (static_cast<unsigned char>(hex) >> 4) & 0xf;
        char lower = (static_cast<unsigned char>(hex) >> 0) & 0xf;
        return base_type::put(sink, '=')
            && base_type::put_nobreak(sink, static_cast<char>((upper < 10) ? upper + '0' : upper + 'A' - 10))
            && base_type::put_nobreak(sink, static_cast<char>((lower < 10) ? lower + '0' : lower + 'A' - 10));
    }

private:
    char prior_ = 0;
};


/*!
  quoted-printable のデコーダ.
  input_filter と output_filter の両方を指定できるが、１つのインスタンスに対してどちらか片方しか使用してはいけない
*/
class qprint_decoder
{
public:
    using char_type = char;
    struct category : boost::iostreams::filter_tag, boost::iostreams::input, boost::iostreams::output {};

    template <typename Source>
    int get(Source & src) const
    {
        int ch = boost::iostreams::get(src);
        if (ch == '=') {
            ch = boost::iostreams::get(src);
            if (ch == EOF || ch == boost::iostreams::WOULD_BLOCK)
                return ch;
            if (ch == '\r' || ch == '\n') {
                if ((ch = boost::iostreams::get(src)) == '\n')
                    ch = boost::iostreams::get(src);
            } else {
                int prior = ch;
                ch = boost::iostreams::get(src);
                if (ch == EOF || ch == boost::iostreams::WOULD_BLOCK)
                    return ch;
                ch = unescape(prior, ch);
            }
        }
        return ch;
    }

    template <typename Sink>
    bool put(Sink & sink, char ch)
    {
        if (tmp[1]) {
            int hex = unescape(tmp[1], ch);
            tmp[0] = tmp[1] = 0;
            return hex < 0 ? false : boost::iostreams::put(sink, static_cast<char>(hex));
        }

        if (tmp[0]) {
            tmp[1] = ch;
            return true;
        }

        if (ch == '=') {
            tmp[0] = '=';
            return true;
        }

        return boost::iostreams::put(sink, ch);
    }

private:
    int unescape(int upper, int lower) const
    {
        int hex = 0;
        if      ('0' <= upper && upper <= '9') hex = upper - '0';
        else if ('a' <= upper && upper <= 'f') hex = upper - 'a';
        else if ('A' <= upper && upper <= 'F') hex = upper - 'A';
        else    return -1;
        hex <<= 4;
        if      ('0' <= lower && lower <= '9') hex += lower - '0';
        else if ('a' <= lower && lower <= 'a') hex += lower - 'a';
        else if ('A' <= lower && lower <= 'A') hex += lower - 'A';
        else    return -1;
        return hex;
    }

private:
    char tmp[2] = { 0, 0 };
};

} }
