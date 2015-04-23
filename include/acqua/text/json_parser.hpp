#pragma once

#include <boost/variant.hpp>
#include <boost/exception/exception.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <acqua/text/parse_error.hpp>
#include <iostream>

namespace acqua { namespace text { namespace detail_json {

template <typename Destinate>
struct any_parser
{
    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        switch(ch) {
            case '\0':
                return true;
            case ' ': case '\t' : case '\r' : case '\n':
                // SkipSpace
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
            case '-': case '+':
            case '1' ... '9':
                next.number(ch);
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
        }

        throw parse_error();
    }
};


template <typename Destinate>
struct literal_parser
{
    enum l_type { l_null, l_true, l_false } l_;
    char const * lit_;

    literal_parser(l_type l)
        : l_(l), lit_(literal()) {}

    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        if (!*++lit_) {
            next.complete();
            return true;
        } else if (std::tolower(ch, std::locale("C")) == *lit_) {
            return true;
        }

        throw parse_error();
    }

    char const * literal()
    {
        switch(l_) {
            case l_null:  return "null";
            case l_true:  return "true";
            case l_false: return "false";
            default:      return "";
        }
    }
};


template <typename Destinate>
struct number_parser
{
    enum n_type { n_decimal, n_fraction, n_exp, n_power } n_ = n_decimal;
    int decimal_;
    int fraction_;
    int power_;
    bool minus_;

    number_parser(int decimal, bool minus = false)
        : decimal_(decimal), fraction_(0), power_(0), minus_(minus) {}

    template <typename CharT, typename Parser>
    bool parse(CharT const & ch, Parser & next)
    {
        switch(n_) {
            case n_decimal:
                if (ch == 'e' || ch == 'E') {
                    if (minus_) decimal_ *= -1;
                    n_ = n_exp;
                    return true;
                }
                if (ch == '.') {
                    if (minus_) decimal_ *= -1;
                    n_ = n_fraction;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + (ch - '0');
                    return true;
                }

                if (minus_) decimal_ *= -1;
                next.complete();
                return false;
            case n_fraction:
                if (ch == 'e' || ch == 'E') {
                    n_ = n_exp;
                    return true;
                }
                if (std::isdigit(ch)) {
                    fraction_ = fraction_ * 10 + ch - '0';
                    return true;
                }

                next.complete();
                return false;
            case n_exp:
                switch(ch) {
                    case '0' ... '9':
                        power_ = ch - '0';
                        //return false;
                    case '+':
                        n_ = n_power;
                        minus_ = false;
                        return true;
                    case '-':
                        n_ = n_power;
                        minus_ = true;
                        return true;
                }
                break;
            case n_power:
                if (std::isdigit(ch)) {
                    power_ = power_ * 10 + ch - '0';
                    return true;
                }

                if (minus_) power_ *= -1;
                next.complete();
                return false;
        }

        throw parse_error();
    }
};


template <typename Destinate, typename String>
struct string_parser
{
    String str_;
    bool escape_ = false;
    char hex_[6];

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
                    case '\r':
                        str_ += '\r';
                        break;
                    case '\t':
                        str_ += '\t';
                        break;
                    case 'u': case 'U':
                        hex_[0] = ch;
                        hex_[1] = 0;
                        return true;
                    default:
                        throw parse_error();
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
                    wchar_t tmp[] = { (wchar_t)std::strtol(hex_ + 1, nullptr, 16), 0 };
                    str_ += boost::locale::conv::utf_to_utf<CharT>(tmp);
                    escape_ = false;
                    return true;
                }
            }
        } else if (ch == '"') {
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

        throw parse_error();
    }
};


template <typename Destinate, typename Parser>
struct array_parser
{
    std::unique_ptr<Parser> data_;
    int index_ = 0;

    template <typename CharT>
    bool parse(CharT const & ch, Parser & next)
    {
        if (data_ && data_->is_progress()) {
            return data_->parse(ch);
        } else if (ch == ']') {
            next.complete();
            return true;
        } else if (!data_) {
            data_.reset(new Parser(next));
            ++index_;
            return data_->parse(ch);
        } else if (ch == ',') {
            data_.reset(new Parser(next));
            ++index_;
            return true;
        }

        throw parse_error();
    }
};


template <typename Destinate, typename Parser>
struct object_parser
{
    std::unique_ptr<Parser> key_;
    std::unique_ptr<Parser> val_;

    template <typename CharT>
    bool parse(CharT const & ch, Parser & next)
    {
        if (val_ && val_->is_progress()) {
            return val_->parse(ch);
        } else if (key_ && key_->is_progress()) {
            return key_->parse(ch);
        } else if (ch == '}') {
            next.complete();
            return true;
        } else if (!key_) {
            if (ch == '"') {
                key_.reset(new Parser(next));
                key_->string();
                return true;
            }
        } else if (!val_) {
            if (ch == ':') {
                val_.reset(new Parser(next));
                return true;
            }
        }

        throw parse_error();
    }
};

template <typename Destinate, typename CharT>
class json_parser
    : public boost::variant<
        any_parser<Destinate>,
        literal_parser<Destinate>,
        number_parser<Destinate>,
        string_parser<Destinate, std::basic_string<CharT> >,
        array_parser<Destinate, json_parser<Destinate, CharT> >,
        object_parser<Destinate, json_parser<Destinate, CharT> >
    >
{
    using any_type  = any_parser<Destinate>;
    using literal_type = literal_parser<Destinate>;
    using number_type = number_parser<Destinate>;
    using string_type = string_parser<Destinate, std::basic_string<CharT> >;
    using array_type = array_parser<Destinate, json_parser<Destinate, CharT> >;
    using object_type = object_parser<Destinate, json_parser<Destinate, CharT> >;
    using base_type = boost::variant<any_type, literal_type, number_type, string_type, array_type, object_type>;

    struct parse_visitor : boost::static_visitor<bool>
    {
        json_parser & next_;
        CharT const & ch_;
        parse_visitor(json_parser & next, CharT const & ch)
            : next_(next), ch_(ch) {}

        template <typename Parser>
        bool operator()(Parser & parser) const
        {
            return parser.parse(ch_, next_);
        }
    };

public:
    json_parser(Destinate * dest)
        : base_type(), dest_(dest) {}

    json_parser(json_parser const & rhs)
        : base_type(), dest_(rhs.dest_) {}

    bool parse(CharT ch)
    {
        return boost::apply_visitor(parse_visitor(*this, ch), *this);
    }

    void null()
    {
        static_cast<base_type &>(*this) = literal_type(literal_type::l_null);
    }

    void boolean(bool flag)
    {
        static_cast<base_type &>(*this) = literal_type( flag ? literal_type::l_true : literal_type::l_false );
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
        is_compl_ = true;
    }

    bool is_progress()
    {
        return !is_compl_;
    }

private:
    Destinate * dest_;
    bool is_compl_ = false;
};

}

template <typename Destinate, typename CharT>
class json_parser
{
    using parser = detail_json::json_parser<Destinate, CharT>;

public:
    json_parser(Destinate & dest)
        : parser_(&dest) {}

    bool parse(CharT ch)
    {
        while(!parser_.parse(ch));
        return !parser_.is_progress();
    }

private:
    parser parser_;
};

} }
