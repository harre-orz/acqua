#pragma once

#include <boost/variant.hpp>
#include <boost/exception/exception.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <iostream>

namespace acqua { namespace text {

struct parse_error : virtual  std::exception, virtual boost::exception {};

template <typename CharT, typename Dest>
class basic_json_parser
{
    struct json_visitor;
    struct any_parser;
    struct literal_parser;
    struct number_parser;
    struct string_parser;
    struct array_parser;
    struct object_parser;
    using Parser = boost::variant<any_parser, literal_parser, number_parser, string_parser, array_parser, object_parser>;

    struct any_parser
    {
        Dest * dest_;
        any_parser(Dest * dest) : dest_(dest) {}

        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " any_parser" << std::endl;

            switch(ch) {
                case '\0':
                    return true;
                case ' ': case '\t' : case '\r' : case '\n':
                    // SkipSpace
                    return true;
                case 'n': case 'N':
                    next = literal_parser(dest_, literal_parser::l_null);
                    return true;
                case 't': case 'T':
                    next = literal_parser(dest_, literal_parser::l_true);
                    return true;
                case 'f': case 'F':
                    next = literal_parser(dest_, literal_parser::l_false);
                    return true;
                case '1' ... '9':
                    next = number_parser(dest_, ch - '0');
                    return true;
                case '-':
                    next = number_parser(dest_, 0, true);
                    return true;
                case '"':
                    next = string_parser(dest_);
                    return true;
                case '[':
                    next = array_parser(dest_);
                    return true;
                case '{':
                    next = object_parser(dest_);
                    return true;
            }

            throw parse_error();
        }
    };

    struct literal_parser
    {
        Dest * dest_;
        enum l_type { l_null, l_true, l_false } l_;
        char const * s_;
        literal_parser(Dest * dest, l_type l) : dest_(dest), l_(l), s_(str()) {}

        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " literal_parser" << std::endl;

            if (*++s_ == '\0') {
                std::cout << "(literal) " << str() << std::endl;
                next = any_parser(dest_);
                return true;
            } else if (std::tolower(ch, std::locale("C")) == *s_) {
                return true;
            }

            throw parse_error();
        }

        char const * str() const
        {
            switch(l_) {
                case l_null:  return "null";
                case l_true:  return "true";
                case l_false: return "false";
                default:      return "";
            }
        }
    };

    struct number_parser
    {
        Dest * dest_;
        enum n_type { n_decimal, n_fraction, n_exp, n_power } n_ = n_decimal;
        int decimal_;
        int fraction_;
        int power_;
        bool minus_;
        number_parser(Dest * dest, int decimal, bool minus = false)
            : dest_(dest), decimal_(decimal), fraction_(0), power_(0), minus_(minus) {}

        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " number_parser" << std::endl;

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
                    std::cout << "(integer) " << decimal_ << std::endl;
                    next = any_parser(dest_);
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

                    std::cout << "(float) " << decimal_ << '.' << fraction_ << std::endl;
                    next = any_parser(dest_);
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
                    std::cout << "(float) " << decimal_ << '.' << fraction_ << 'e' << power_ << std::endl;
                    next = any_parser(dest_);
                    return false;
            }

            throw parse_error();
        }
    };

    struct string_parser
    {
        Dest * dest_;
        std::basic_string<CharT> s_;
        bool escape_ = false;
        char c_[6];
        string_parser(Dest * dest) : dest_(dest) {}
        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " string_parser" << std::endl;

            if (escape_) {
                if (c_[0] == 0) {
                    switch(ch) {
                        case '"':
                            s_ += '"';
                            break;
                        case '\\':
                            s_ += '\\';
                            break;
                        case '/':
                            s_ += '/';
                            break;
                        case 'b':
                            s_ += '\b';
                            break;
                        case 'f':
                            s_ += '\f';
                            break;
                        case 'n':
                            s_ += '\n';
                            break;
                        case '\r':
                            s_ += '\r';
                            break;
                        case '\t':
                            s_ += '\t';
                            break;
                        case 'u': case 'U':
                            c_[0] = ch;
                            c_[1] = 0;
                            return true;
                        default:
                            throw parse_error();
                    }

                    escape_ = false;
                    return true;
                } else if (c_[1] == 0) {
                    if (std::isxdigit(ch)) {
                        c_[1] = ch;
                        c_[2] = 0;
                        return true;
                    }
                } else if (c_[2] == 0) {
                    if (std::isxdigit(ch)) {
                        c_[2] = ch;
                        c_[3] = 0;
                        return true;
                    }
                } else if (c_[3] == 0) {
                    if (std::isxdigit(ch)) {
                        c_[3] = ch;
                        c_[4] = 0;
                        return true;
                    }
                } else if (c_[4] == 0) {
                    if (std::isxdigit(ch)) {
                        c_[4] = ch;
                        c_[5] = 0;
                        wchar_t tmp[] = { (wchar_t)std::strtol(c_ + 1, nullptr, 16), 0 };
                        s_ += boost::locale::conv::utf_to_utf<CharT>(tmp);
                        escape_ = false;
                        return true;
                    }
                }
            } else if (ch == '"') {
                std::cout << "(string) " << '"' << s_ << '"' << std::endl;
                next = any_parser(dest_);
                return true;
            } else if (ch == '\\') {
                escape_ = true;
                c_[0] = 0;
                return true;
            } else if (std::isprint(ch, std::locale("C"))) {
                s_ += ch;
                return true;
            }

            throw parse_error();
        }
    };

    struct array_parser
    {
        Dest * dest_;
        std::unique_ptr<Parser> data_;
        int index_ = 0;

        array_parser(Dest * dest) : dest_(dest) {}
        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " array_parser" << std::endl;

            if (data_ && data_->which() != 0) {
                return boost::apply_visitor(json_visitor(ch, *data_), *data_);
            } else if (ch == ']') {
                next = any_parser(dest_);
                return true;
            } else if (!data_) {
                data_.reset(new Parser(any_parser(dest_)));
                ++index_;
                return boost::apply_visitor(json_visitor(ch, *data_), *data_);
            } else if (ch == ',') {
                data_.release();
                ++index_;
                return true;
            }

            throw parse_error();
        }
    };

    struct object_parser
    {
        Dest * dest_;
        std::unique_ptr<Parser> key_;
        std::unique_ptr<Parser> val_;

        object_parser(Dest * dest) : dest_(dest) {}
        bool put(CharT const & ch, Parser & next)
        {
            std::cout << ch << " object_parser" << std::endl;

            if (!key_ && val_ && val_->which() == 0) {
                key_.reset(new Parser(string_parser(dest_)));
                return boost::apply_visitor(json_visitor(ch, *val_), *val_);
            } else if (val_ && val_->which() != 0) {
                return boost::apply_visitor(json_visitor(ch, *val_), *val_);
            } else if (key_ && key_->which() != 0) {
                return boost::apply_visitor(json_visitor(ch, *key_), *key_);
            } else if (ch == '}') {
                std::cout << "(object)" << std::endl;
                next = any_parser(dest_);
                return true;
            } else {
                if (key_->which() == 0 && ch == ':') {
                    key_.release();
                    val_.reset(new Parser(any_parser(dest_)));
                    return true;
                }
            }

            throw parse_error();
        }
    };

    struct json_visitor : boost::static_visitor<bool>
    {
        CharT const & ch_;
        Parser & next_;
        json_visitor(CharT const & ch, Parser & next)
            : ch_(ch), next_(next) {}

        template <typename Parser>
        bool operator()(Parser & parser) const
        {
            return parser.put(ch_, next_);
        }
    };

public:
    explicit basic_json_parser(Dest & dest)
        : data_( any_parser(&dest) )
    {
    }

    bool parse(CharT ch)
    {
        while(!boost::apply_visitor(json_visitor(ch, data_), data_))
            ;
        return data_.which() == 0;
    }

private:
    Parser data_;
};

} }
