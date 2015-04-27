/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/variant.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <acqua/text/adapt/parse_adaptor.hpp>

namespace acqua { namespace text {

namespace json_impl {

using acqua::text::adapt::parse_adaptor;

enum literal { l_null, l_true, l_false };

class any_parser
{
public:
    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        switch(ch) {
            case '\0':
                return true;
            case ' ': case '\t': case '\r': case '\n':
                return true;
            case 'n': case 'N':
                next.null();
                return true;
            case 't': case 'T':
                next.boolean(true);
                return true;
            case 'f': case 'F':
                next.boolean(false);
                return true;
            case '"':
                next.string();
                return true;
            case '[':
                next.array();
                return true;
            case '{':
                next.object();
                return true;
            // case '-': case '+':
            // case '0' ... '9':
            default:
                next.number(ch);
                return true;
        }

        next.error();
        return true;
    }
};


class literal_parser
{
private:
    literal l_;
    char const * lit_;

    char const * c_str() const
    {
        switch(l_) {
            case l_null:  return "null";
            case l_true:  return "true";
            case l_false: return "false";
            default:      return "";
        }
    }

public:
    literal_parser(literal l)
        : l_(l), lit_(c_str()) {}

    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        if (*++lit_ == '\0') {
            switch(l_) {
                case l_null:
                    parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_value(nullptr);
                    break;
                case l_true:
                    parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_value(true);
                    break;
                case l_false:
                    parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_value(false);
                    break;
            }
            next.complete();
            return false;
        } else if (std::tolower(ch, std::locale("C")) == *lit_) {
            return true;
        }

        next.error();
        return true;
    }
};


class number_parser
{
    enum n_type { n_decimal, n_fraction, n_exp, n_exponent } n_ = n_decimal;
    long decimal_;
    int fraction_;
    int exponent_;
    bool sign_;

public:
    number_parser(int decimal, bool sign = false)
        : decimal_(decimal), fraction_(1), exponent_(0), sign_(sign) {}

    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        switch(n_) {
            case n_decimal:
                if (ch == 'e' || ch == 'E') {
                    if (sign_) decimal_ *= -1;
                    n_ = n_exp;
                    return true;
                }
                if (ch == '.') {
                    n_ = n_fraction;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + (ch - '0');
                    return true;
                }

                if (sign_) decimal_ *= -1;
                parse_adaptor<typename Parser::destinate_type>(next.destinate())
                    .parse_value(decimal_);
                next.complete();
                return false;
            case n_fraction:
                if (ch == 'e' || ch == 'E') {
                    if (sign_) decimal_ *= -1;
                    n_ = n_exp;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + ch - '0';
                    fraction_ *= 10;
                    return true;
                }

                if (sign_) decimal_ *= -1;
                parse_adaptor<typename Parser::destinate_type>(next.destinate())
                    .parse_value((double)decimal_ / (double)fraction_);
                next.complete();
                return false;
            case n_exp:
                switch(ch) {
                    case '1' ... '9':
                        exponent_ = ch - '0';
                        //return false;
                    case '+': case '0':
                        n_ = n_exponent;
                        sign_ = false;
                        return true;
                    case '-':
                        n_ = n_exponent;
                        sign_ = true;
                        return true;
                }
                break;
            case n_exponent:
                if (std::isdigit(ch)) {
                    exponent_ = exponent_ * 10 + (ch - '0');
                    return true;
                }
                if (sign_) exponent_ *= -1;
                parse_adaptor<typename Parser::destinate_type>(next.destinate())
                    .parse_value((double)decimal_ * std::pow(10, exponent_) / (double)fraction_);
                next.complete();
                return false;
        }

        next.error();
        return true;
    }
};


template <typename String>
class string_parser
{
    bool escape_ = false;
    char hex_[6];

protected:
    String str_;

private:
    static void unescape(char16_t ch, std::basic_string<char16_t> & s)
    {
        s += ch;
    }

    template <typename Ch>
    static void unescape(char16_t ch, std::basic_string<Ch> & s)
    {
        namespace utf = boost::locale::utf;

        char16_t * it = &ch;
        utf::code_point cp = utf::utf_traits<char16_t>::decode(it, it + 1);
        if (cp != utf::illegal && cp != utf::incomplete)
            utf::utf_traits<Ch>::encode(cp, std::back_inserter(s));
    }

public:
    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        if (escape_) {
            if (hex_[0] == 0) {
                switch(ch) {
                    case '"':
                        str_ += '"';
                        break;
                    case '\\':
                        str_ += '\\';
                        break;
                    case '/':
                        str_ += '/';
                        break;
                    case 'b':
                        str_ += '\b';
                        break;
                    case 'f':
                        str_ += '\f';
                        break;
                    case 'n':
                        str_ += '\n';
                        break;
                    case 'r':
                        str_ += '\r';
                        break;
                    case 't':
                        str_ += '\t';
                        break;
                    case 'u': case 'U':
                        hex_[0] = ch;
                        hex_[1] = 0;
                        return true;
                    default:
                        next.error();
                        return true;
                }
                escape_ = false;
                return true;
            } else if (hex_[1] == 0) {
                if (std::isxdigit(ch)) {
                    hex_[1] = ch;
                    hex_[2] = 0;
                    return true;
                }
            } else if (hex_[2] == 0) {
                if (std::isxdigit(ch)) {
                    hex_[2] = ch;
                    hex_[3] = 0;
                    return true;
                }
            } else if (hex_[3] == 0) {
                if (std::isxdigit(ch)) {
                    hex_[3] = ch;
                    hex_[4] = 0;
                    return true;
                }
            } else if (hex_[4] == 0) {
                if (std::isxdigit(ch)) {
                    hex_[4] = ch;
                    hex_[5] = 0;
                    escape_ = false;
                    unescape(std::strtol(hex_ + 1, nullptr, 16), str_);
                    return true;
                }
            }
        } else if (ch == '"') {
            parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_value(str_);
            next.complete();
            return true;
        } else if (ch == '\\') {
            escape_ = true;
            hex_[0] = 0;
            return true;
        } else if (std::isprint(ch, std::locale("C"))) {
            str_ += ch;
            return true;
        }

        next.error();
        return true;
    }
};


template <typename Parser>
class array_parser
{
    std::unique_ptr<Parser> data_;
    int index_ = 0;

public:
    template <typename CharT>
    bool parse(CharT const & ch, Parser & next)
    {
        if (data_ && data_->is_progress()) {
            return data_->parse(ch);
        } else if (ch == ']') {
            next.complete();
            return true;
        } else if (!data_) {
            data_.reset(new Parser(parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_child(index_++)));
            return data_->parse(ch);
        } else if (ch == ',') {
            data_.reset(new Parser(parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_child(index_++)));
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        next.error();
        return true;
    }
};


template <typename Parser, typename String>
class object_parser
{
    struct string : string_parser<String>
    {
        using destinate_type = String;
        bool ready_ = false;
        bool compl_ = false;
        void ready()
        {
            ready_ = true;
        }
        bool is_ready() const
        {
            return ready_;
        }
        void complete()
        {
            compl_ = true;
        }
        void error()
        {
            // FIXME: このエラーをハンドリングしていない
            compl_ = true;
        }
        bool is_progress() const
        {
            return !compl_;
        }
        destinate_type & destinate()
        {
            return this->str_;
        }
        void clear()
        {
            ready_ = false;
            compl_ = false;
            this->str_.clear();
        }
    } key_;
    std::unique_ptr<Parser> val_;
    bool first_ = true;

public:
    template <typename CharT>
    bool parse(CharT const & ch, Parser & next)
    {
        if (val_) {
            if (val_->is_progress())
                return val_->parse(ch);
            if (ch == '}') {
                next.complete();
                return true;
            }
            if (ch == ',') {
                key_.clear();
                val_.release();
                return true;
            }
        } else if (!key_.is_ready()) {
            if (first_ && ch == '}') {
                next.complete();
                return true;
            }
            if (ch == '"') {
                first_ = false;
                key_.ready();
                return true;
            }
        } else if (key_.is_progress()) {
            return key_.parse(ch, key_);
        } else if (ch == ':') {
            val_.reset(new Parser(parse_adaptor<typename Parser::destinate_type>(next.destinate()).parse_child(key_.destinate())));
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        next.error();
        return true;
    }
};


template <typename CharT, typename Destinate>
class feed_parser
    : public boost::variant< any_parser, literal_parser, number_parser,
                             string_parser< std::basic_string<CharT> >,
                             array_parser< feed_parser<CharT, Destinate> >,
                             object_parser< feed_parser<CharT, Destinate>, std::basic_string<CharT> > >
{
    using any_type  = any_parser;
    using literal_type = literal_parser;
    using number_type = number_parser;
    using string_type = string_parser< std::basic_string<CharT> >;
    using array_type = array_parser< feed_parser<CharT, Destinate> >;
    using object_type = object_parser< feed_parser<CharT, Destinate>, std::basic_string<CharT> >;
    using base_type = boost::variant<any_type, literal_type, number_type, string_type, array_type, object_type>;

    struct parse_visitor : boost::static_visitor<bool>
    {
        feed_parser & next_;
        CharT const & ch_;
        parse_visitor(feed_parser & next, CharT const & ch)
            : next_(next), ch_(ch) {}

        template <typename Parser>
        bool operator()(Parser & data) const
        {
            return data.parse(ch_, next_);
        }
    };

public:
    using char_type = CharT;
    using destinate_type = Destinate;

public:
    feed_parser(Destinate & dest)
        : base_type(), dest_(&dest) {}

    feed_parser(feed_parser const & rhs)
        : base_type(), dest_(rhs.dest_) {}

    destinate_type & destinate()
    {
        return *dest_;
    }

    bool parse(CharT ch)
    {
        return boost::apply_visitor(parse_visitor(*this, ch), *this);
    }

    void null()
    {
        static_cast<base_type &>(*this) = literal_type(l_null);
    }

    void boolean(bool val)
    {
        static_cast<base_type &>(*this) = literal_type(val ? l_true : l_false);
    }

    void number(char ch)
    {
        static_cast<base_type &>(*this)
            = (ch == '-')
            ? number_type(0, true)
            : (ch == '+')
            ? number_type(0, false)
            : number_type(ch - '0', false);
    }

    void string()
    {
        static_cast<base_type &>(*this) = string_type();
    }

    void array()
    {
        static_cast<base_type &>(*this) = array_type();
    }

    void object()
    {
        static_cast<base_type &>(*this) = object_type();
    }

    void complete()
    {
        static_cast<base_type &>(*this) = any_type();
        compl_ = true;
    }

    void error()
    {
        complete();
        error_ = true;
    }

    bool is_progress() const
    {
        return !compl_;
    }

private:
    destinate_type * dest_;
    bool compl_ = false;
    bool error_ = false;
};

} // json_impl

template <typename CharT, typename Destinate>
class json_feed_parser
{
    using data_type = json_impl::feed_parser<CharT, Destinate>;

public:
    json_feed_parser(Destinate & dest)
        : data_(dest) {}

    bool good() const
    {
        return data_.is_progress();
    }

    json_feed_parser & operator<<(CharT ch)
    {
        while(!data_.parse(ch));
        return *this;
    }

    json_feed_parser & operator<<(CharT const * str)
    {
        while(*str)
            *this << *str++;
        return *this;
    }

    json_feed_parser & operator<<(std::basic_string<CharT> const & str)
    {
        for(CharT const & ch : str)
            *this << ch;
        return *this;
    }

private:
    data_type data_;
};

} }
