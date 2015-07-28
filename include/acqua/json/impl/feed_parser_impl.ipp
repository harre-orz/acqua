/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <limits>
#include <memory>
#include <boost/variant.hpp>
#include <boost/locale.hpp>
#include <acqua/json/feed_parser.hpp>

namespace acqua { namespace json { namespace detail {

enum literal { l_null, l_true, l_false };

class any_parser
{
public:
    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        switch(ch) {
            case '\0':
                return true;
            case ' ': case '\t': case '\r': case '\n':
                return true;
            case 'n': case 'N':
                impl.to_null();
                return true;
            case 't': case 'T':
                impl.to_bool(true);
                return true;
            case 'f': case 'F':
                impl.to_bool(false);
                return true;
            case '"':
                impl.to_string();
                return true;
            case '[':
                impl.to_array();
                return true;
            case '{':
                impl.to_object();
                return true;
            // case '-': case '+':
            // case '0' ... '9':
            default:
                impl.to_number(ch);
                return true;
        }
        impl.failure();
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

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (*++lit_ == '\0') {
            switch(l_) {
                case l_null:
                    impl.to_terminated(nullptr);
                    break;
                case l_true:
                    impl.to_terminated(true);
                    break;
                case l_false:
                    impl.to_terminated(false);
                    break;
            }
            return false;
        } else if (std::tolower(ch, std::locale::classic()) == *lit_) {
            return true;
        }
        impl.failure();
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

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
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
                impl.to_terminated(decimal_);
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
                impl.to_terminated((double)decimal_ / (double)fraction_);
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
                impl.to_terminated((double)decimal_ * std::pow(10, exponent_) / (double)fraction_);
                return false;
        }

        impl.failure();
        return true;
    }
};


template <typename String>
class string_parser
{
public:
    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
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
                        impl.failure();
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
            impl.to_terminated(std::move(str_));
            return true;
        } else if (ch == '\\') {
            escape_ = true;
            hex_[0] = 0;
            return true;
        } else if (std::isprint(ch, std::locale::classic())) {
            str_ += ch;
            return true;
        }

        impl.failure();
        return true;
    }

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

private:
    bool escape_ = false;
    char hex_[6];

protected:
    String str_;
};


template <typename Impl>
class array_parser
{
    std::unique_ptr<Impl> data_;
    int index_ = 0;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (data_ && !data_->is_terminated()) {
            return data_->do_parse_1(ch);
        } else if (ch == ']') {
            impl.to_terminated();
            return true;
        } else if (!data_) {
            impl.to_child(index_++, data_);
            return data_->do_parse_1(ch);
        } else if (ch == ',') {
            impl.to_child(index_++, data_);
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        impl.failure();
        return true;
    }
};


template <typename Impl, typename String>
class object_parser
{
    struct string : string_parser<String>
    {
        explicit string(Impl & parent) : parent_(parent) {}
        bool is_terminated() const { return is_terminated_; }
        template <typename T>
        void to_terminated(T const &) { is_terminated_ = true; }
        void failure() { parent_.failure(); }
        Impl & parent_;
        bool is_terminated_ = false;
        using string_parser<String>::str_;
    };
    std::unique_ptr<string> key_;
    std::unique_ptr<Impl> val_;
    bool first_ = true;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (val_) {
            if (!val_->is_terminated())
                return val_->do_parse_1(ch);
            if (ch == '}') {
                impl.to_terminated();
                return true;
            }
            if (ch == ',') {
                key_.release();
                val_.release();
                return true;
            }
        } else if (!key_) {
            if (first_ && ch == '}') {
                impl.to_terminated();
                return true;
            }
            if (ch == '"') {
                first_ = false;
                key_.reset(new string(impl));
                return true;
            }
        } else if (!key_->is_terminated()) {
            return (*key_)(ch, *key_);
        } else if (ch == ':') {
            impl.to_child(std::move(key_->str_), val_);
            return true;
        }
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;
        impl.failure();
        return true;
    }
};

}

using namespace acqua::json::detail;

template <typename Json, typename Adapt, typename CharT>
class feed_parser<Json, Adapt, CharT>::impl
    : private Adapt
    , public boost::variant<any_parser, literal_parser, number_parser,
                            string_parser<std::basic_string<CharT> >,
                            array_parser<impl>, object_parser<impl, std::basic_string<CharT> >
                            >
{
private:
    using any = any_parser;
    using lit = literal_parser;
    using num = number_parser;
    using str = string_parser<std::basic_string<CharT> >;
    using arr = array_parser<impl>;
    using obj = object_parser<impl, std::basic_string<CharT> >;
    using base_type = boost::variant<any, lit, num, str, arr, obj>;

    struct feed_visitor : boost::static_visitor<bool>
    {
        CharT const & ch_;
        impl & impl_;
        feed_visitor(CharT const & ch, impl & impl)
            : ch_(ch), impl_(impl) {}
        template <typename Parser>
        bool operator()(Parser & parser) const
        {
            return parser(ch_, impl_);
        }
    };

public:
    impl(boost::system::error_code & error, Json & json)
        : Adapt(json), error_(error)
    {
    }

    bool is_terminated() const
    {
        return is_terminated_;
    }

    bool do_parse_1(CharT ch)
    {
        return boost::apply_visitor(feed_visitor(ch, *this), *this);
    }

    void to_null()
    {
        *static_cast<base_type *>(this) = lit(literal::l_null);
    }

    void to_bool(bool val)
    {
        *static_cast<base_type *>(this) = lit(val ? literal::l_true : literal::l_false);
    }

    void to_number(char ch)
    {
        *static_cast<base_type *>(this)
            = (ch == '-') ? num(0, true)
            : (ch == '+') ? num(0, false)
            :               num(ch - '0', false);
    }

    void to_string()
    {
        *static_cast<base_type *>(this) = str();
    }

    void to_array()
    {
        *static_cast<base_type *>(this) = arr();
    }

    void to_object()
    {
        *static_cast<base_type *>(this) = obj();
    }

    template <typename T>
    void to_child(T const & t, std::unique_ptr<impl> & impl_)
    {
        impl_.reset(new impl(error_, static_cast<Adapt *>(this)->add_child(t)));
    }

    template <typename T>
    void to_child(T && t, std::unique_ptr<impl> & impl_)
    {
        impl_.reset(new impl(error_, static_cast<Adapt *>(this)->add_child(std::move(t))));
    }

    void to_terminated()
    {
        *static_cast<base_type *>(this) = any();
        is_terminated_ = true;
    }

    template <typename T>
    void to_terminated(T const & t)
    {
        static_cast<Adapt *>(this)->data(t);
        to_terminated();
    }

    template <typename T>
    void to_terminated(T && t)
    {
        static_cast<Adapt *>(this)->data(std::move(t));
        to_terminated();
    }

    void failure()
    {
        // TODO: 適切なエラーコードとメッセージを定義する
        error_.assign(EIO, boost::system::generic_category());
    }

private:
    boost::system::error_code & error_;
    bool is_terminated_ = false;
};

} }
