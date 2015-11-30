#pragma once

#include <cmath>
#include <boost/variant.hpp>
#include <boost/locale/encoding_utf.hpp>

namespace acqua { namespace iostreams { namespace detail {

enum class json_literal { l_null, l_true, l_false };
enum class json_sign { plus, minus };

class json_any_parser
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
                impl.new_literal(json_literal::l_null);
                return true;
            case 't': case 'T':
                impl.new_literal(json_literal::l_true);
                return true;
            case 'f': case 'F':
                impl.new_literal(json_literal::l_false);
                return true;
            case '"':
                impl.new_string();
                return true;
            case '[':
                impl.new_array();
                return true;
            case '{':
                impl.new_object();
                return true;
            // case '-': case '+':
            // case '0' ... '9':
            default:
                impl.new_number(ch);
                return true;
        }
        impl.failure(this, ch);
        return true;
    }
};


class json_literal_parser
{
private:
    json_literal l_;
    char const * lit_;

    static char const * c_str(json_literal l)
    {
        switch(l) {
            case json_literal::l_null:  return "null";
            case json_literal::l_true:  return "true";
            case json_literal::l_false: return "false";
            default:                    return "";
        }
    }

public:
    json_literal_parser(json_literal l)
        : l_(l), lit_(c_str(l)) {}

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (*++lit_ == '\0') {
            switch(l_) {
                case json_literal::l_null:
                    impl.determined(nullptr);
                    break;
                case json_literal::l_true:
                    impl.determined(true);
                    break;
                case json_literal::l_false:
                    impl.determined(false);
                    break;
            }
            return false;
        } else if (std::tolower(ch, std::locale::classic()) == *lit_) {
            return true;
        }
        impl.failure(this, ch);
        return true;
    }
};


class json_number_parser
{
    enum class n_type { decimal, fraction, exp, exponent } n_ = n_type::decimal;
    long decimal_;
    int fraction_;
    int exponent_;
    json_sign sign_;

public:
    json_number_parser(int decimal, enum json_sign sign)
        : decimal_(decimal), fraction_(1), exponent_(0), sign_(sign) {}

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        switch(n_) {
            case n_type::decimal:
                if (ch == 'e' || ch == 'E') {
                    if (sign_ == json_sign::minus) decimal_ *= -1;
                    n_ = n_type::exp;
                    return true;
                }
                if (ch == '.') {
                    n_ = n_type::fraction;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + (ch - '0');
                    return true;
                }

                if (sign_ == json_sign::minus) decimal_ *= -1;
                impl.determined(decimal_);
                return false;
            case n_type::fraction:
                if (ch == 'e' || ch == 'E') {
                    if (sign_ == json_sign::minus) decimal_ *= -1;
                    n_ = n_type::exp;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + ch - '0';
                    fraction_ *= 10;
                    return true;
                }

                if (sign_ == json_sign::minus) decimal_ *= -1;
                impl.determined(static_cast<double>(decimal_) / static_cast<double>(fraction_));
                return false;
            case n_type::exp:
                switch(ch) {
                    case '1' ... '9':
                        exponent_ = ch - '0';
                        //return false;
                    case '+': case '0':
                        n_ = n_type::exponent;
                        sign_ = json_sign::plus;
                        return true;
                    case '-':
                        n_ = n_type::exponent;
                        sign_ = json_sign::minus;
                        return true;
                }
                break;
            case n_type::exponent:
                if (std::isdigit(ch)) {
                    exponent_ = exponent_ * 10 + (ch - '0');
                    return true;
                }
                if (sign_ == json_sign::minus) exponent_ *= -1;
                impl.determined(static_cast<double>(decimal_) * std::pow(10, exponent_) / static_cast<double>(fraction_));
                return false;
        }

        impl.failure(this, ch);
        return true;
    }
};


template <typename String>
class json_string_parser
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
                        impl.failure(this, ch);
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
                    unescape(static_cast<char16_t>(std::strtol(hex_ + 1, nullptr, 16)), str_);
                    return true;
                }
            }
        } else if (ch == '"') {
            impl.determined(std::move(str_));
            return true;
        } else if (ch == '\\') {
            escape_ = true;
            hex_[0] = 0;
            return true;
        } else if (std::isprint(ch, std::locale::classic())) {
            str_ += ch;
            return true;
        }

        impl.failure(this, ch);
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
    String str_ = {};
};


template <typename Impl>
class json_array_parser
{
    std::unique_ptr<Impl> data_ = {};
    int index_ = 0;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (data_ && data_->is_in_progress) {
            return data_->parse_1(ch);
        } else if (ch == ']') {
            impl.determined();
            return true;
        } else if (!data_) {
            impl.new_child(index_++, data_);
            return data_->parse_1(ch);
        } else if (ch == ',') {
            impl.new_child(index_++, data_);
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        impl.failure(this, ch);
        return true;
    }
};


template <typename Impl, typename String>
class json_object_parser
{
    struct string_impl : json_string_parser<String>
    {
        explicit string_impl(Impl & parent) : parent_(parent) {}
        template <typename T>
        void determined(T &&) { is_in_progress = false; }
        template <typename Parser, typename CharT>
        void failure(Parser * p, CharT ch) { parent_.failure(p, ch); }
        Impl & parent_;
        bool is_in_progress = true;
        using json_string_parser<String>::str_;
    };
    std::unique_ptr<string_impl> key_ = {};
    std::unique_ptr<Impl> val_ = {};
    bool first_ = true;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (val_) {
            if (val_->is_in_progress)
                return val_->parse_1(ch);
            if (ch == '}') {
                impl.determined();
                return true;
            }
            if (ch == ',') {
                key_.release();
                val_.release();
                return true;
            }
        } else if (!key_) {
            if (first_ && ch == '}') {
                impl.determined();
                return true;
            }
            if (ch == '"') {
                first_ = false;
                key_.reset(new string_impl(impl));
                return true;
            }
        } else if (key_->is_in_progress) {
            return (*key_)(ch, *key_);
        } else if (ch == ':') {
            impl.new_child(std::move(key_->str_), val_);
            return true;
        }
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;
        impl.failure(this, ch);
        return true;
    }
};

template <typename Json, typename Adapt, typename CharT>
struct json_parser_impl
    : private Adapt
    , public boost::variant<
        json_any_parser, json_literal_parser, json_number_parser,
        json_string_parser<std::basic_string<CharT> >,
        json_array_parser<json_parser_impl<Json, Adapt, CharT> >,
        json_object_parser<json_parser_impl<Json, Adapt, CharT>, std::basic_string<CharT> >
    >
{
    using any = json_any_parser;
    using lit = json_literal_parser;
    using num = json_number_parser;
    using str = json_string_parser<std::basic_string<CharT> >;
    using arr = json_array_parser<json_parser_impl >;
    using obj = json_object_parser<json_parser_impl, std::basic_string<CharT> >;
    using base_type = boost::variant<any, lit, num, str, arr, obj>;

    struct json_visitor : boost::static_visitor<bool>
    {
        CharT const & ch_;
        json_parser_impl & impl_;

        explicit json_visitor(CharT const & ch, json_parser_impl & impl)
            : ch_(ch), impl_(impl) {}

        template <typename Parser>
        bool operator()(Parser & parser) const { return parser(ch_, impl_); }
    };

public:
    json_parser_impl(boost::system::error_code & error, Json & json)
        : Adapt(json), error_(error) {}

    bool parse_1(CharT const & ch)
    {
        return boost::apply_visitor(json_visitor(ch, *this), *this);
    }

    void new_literal(json_literal l)
    {
        *static_cast<base_type *>(this) = lit(l);
    }

    void new_number(char ch)
    {
        *static_cast<base_type *>(this)
            = (ch == '-') ? num(0, json_sign::minus)
            : (ch == '+') ? num(0, json_sign::plus)
            :               num(ch - '0', json_sign::plus);
    }

    void new_string()
    {
        *static_cast<base_type *>(this) = str();
    }

    void new_array()
    {
        *static_cast<base_type *>(this) = arr();
    }

    void new_object()
    {
        *static_cast<base_type *>(this) = obj();
    }

    template <typename T>
    void new_child(T && t, std::unique_ptr<json_parser_impl> & impl)
    {
        impl.reset(new json_parser_impl(error_, static_cast<Adapt *>(this)->add_child(std::forward<T>(t))));
    }

    void determined()
    {
        *static_cast<base_type *>(this) = any();
        is_in_progress = false;
    }

    template <typename T>
    void determined(T && t)
    {
        static_cast<Adapt *>(this)->data(std::forward<T>(t));
        determined();
    }

    template <typename Parser>
    void failure(Parser *, CharT)
    {
    }

private:
    boost::system::error_code & error_;

public:
    bool is_in_progress = true;
};

} } }
