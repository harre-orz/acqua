#pragma once

#include <boost/variant.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <acqua/iostreams/json_parser.hpp>

namespace acqua { namespace iostreams {

namespace json {

enum class literal { l_null, l_true, l_false };
enum class sign { plus, minus };

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
                impl.new_literal(literal::l_null);
                return true;
            case 't': case 'T':
                impl.new_literal(literal::l_true);
                return true;
            case 'f': case 'F':
                impl.new_literal(literal::l_false);
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


class literal_parser
{
private:
    literal l_;
    char const * lit_;

    static char const * c_str(literal l)
    {
        switch(l) {
            case literal::l_null:  return "null";
            case literal::l_true:  return "true";
            case literal::l_false: return "false";
            default:               return "";
        }
    }

public:
    literal_parser(literal l)
        : l_(l), lit_(c_str(l)) {}

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        if (*++lit_ == '\0') {
            switch(l_) {
                case literal::l_null:
                    impl.completed(nullptr);
                    break;
                case literal::l_true:
                    impl.completed(true);
                    break;
                case literal::l_false:
                    impl.completed(false);
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


class number_parser
{
    enum class n_type { decimal, fraction, exp, exponent } n_ = n_type::decimal;
    long decimal_;
    int fraction_;
    int exponent_;
    sign sign_;

public:
    number_parser(int decimal, enum sign sign)
        : decimal_(decimal), fraction_(1), exponent_(0), sign_(sign) {}

    template <typename CharT, typename Impl>
    bool operator()(CharT const & ch, Impl & impl)
    {
        switch(n_) {
            case n_type::decimal:
                if (ch == 'e' || ch == 'E') {
                    if (sign_ == sign::minus) decimal_ *= -1;
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

                if (sign_ == sign::minus) decimal_ *= -1;
                impl.completed(decimal_);
                return false;
            case n_type::fraction:
                if (ch == 'e' || ch == 'E') {
                    if (sign_ == sign::minus) decimal_ *= -1;
                    n_ = n_type::exp;
                    return true;
                }
                if (std::isdigit(ch)) {
                    decimal_ = decimal_ * 10 + ch - '0';
                    fraction_ *= 10;
                    return true;
                }

                if (sign_ == sign::minus) decimal_ *= -1;
                impl.completed(static_cast<double>(decimal_) / static_cast<double>(fraction_));
                return false;
            case n_type::exp:
                switch(ch) {
                    case '1' ... '9':
                        exponent_ = ch - '0';
                        //return false;
                    case '+': case '0':
                        n_ = n_type::exponent;
                        sign_ = sign::plus;
                        return true;
                    case '-':
                        n_ = n_type::exponent;
                        sign_ = sign::minus;
                        return true;
                }
                break;
            case n_type::exponent:
                if (std::isdigit(ch)) {
                    exponent_ = exponent_ * 10 + (ch - '0');
                    return true;
                }
                if (sign_ == sign::minus) exponent_ *= -1;
                impl.completed(static_cast<double>(decimal_) * std::pow(10, exponent_) / static_cast<double>(fraction_));
                return false;
        }

        impl.failure(this, ch);
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
            impl.completed(std::move(str_));
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
class array_parser
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
            impl.completed();
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
class object_parser
{
    struct string_impl : string_parser<String>
    {
        explicit string_impl(Impl & parent) : parent_(parent) {}
        template <typename T>
        void completed(T &&) { is_in_progress = false; }
        template <typename Parser, typename CharT>
        void failure(Parser * p, CharT ch) { parent_.failure(p, ch); }
        Impl & parent_;
        bool is_in_progress = true;
        using string_parser<String>::str_;
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
                impl.completed();
                return true;
            }
            if (ch == ',') {
                key_.release();
                val_.release();
                return true;
            }
        } else if (!key_) {
            if (first_ && ch == '}') {
                impl.completed();
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
struct parser_impl
    : private Adapt
    , public boost::variant<
        any_parser, literal_parser, number_parser,
        string_parser<std::basic_string<CharT> >,
        array_parser<parser_impl<Json, Adapt, CharT> >,
        object_parser<parser_impl<Json, Adapt, CharT>, std::basic_string<CharT> >
    >
{
    using any = any_parser;
    using lit = literal_parser;
    using num = number_parser;
    using str = string_parser<std::basic_string<CharT> >;
    using arr = array_parser<parser_impl >;
    using obj = object_parser<parser_impl, std::basic_string<CharT> >;
    using base_type = boost::variant<any, lit, num, str, arr, obj>;

    struct visitor : boost::static_visitor<bool>
    {
        CharT const & ch_;
        parser_impl & impl_;

        explicit visitor(CharT const & ch, parser_impl & impl)
            : ch_(ch), impl_(impl) {}

        template <typename Parser>
        bool operator()(Parser & parser) const { return parser(ch_, impl_); }
    };

public:
    parser_impl(boost::system::error_code & error, Json & json)
        : Adapt(json), error_(error) {}

    bool parse_1(CharT const & ch)
    {
        return boost::apply_visitor(visitor(ch, *this), *this);
    }

    void new_literal(literal l)
    {
        *static_cast<base_type *>(this) = lit(l);
    }

    void new_number(char ch)
    {
        *static_cast<base_type *>(this)
            = (ch == '-') ? num(0, sign::minus)
            : (ch == '+') ? num(0, sign::plus)
            :               num(ch - '0', sign::plus);
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
    void new_child(T && t, std::unique_ptr<parser_impl> & impl)
    {
        impl.reset(new parser_impl(error_, static_cast<Adapt *>(this)->add_child(std::forward<T>(t))));
    }

    void completed()
    {
        *static_cast<base_type *>(this) = any();
        is_in_progress = false;
    }

    template <typename T>
    void completed(T && t)
    {
        static_cast<Adapt *>(this)->data(std::forward<T>(t));
        completed();
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

}  // json

template <typename Json, typename Adapt, typename CharT>
struct json_parser<Json, Adapt, CharT>::impl
    : json::parser_impl<Json, Adapt, CharT>
{
    impl(boost::system::error_code & error, Json & json)
        : json::parser_impl<Json, Adapt, CharT>(error, json) {}
};


template <typename Json, typename Adapt, typename CharT>
inline json_parser<Json, Adapt, CharT>::json_parser(Json & json)
    : error_(), impl_(new impl(error_, json)) {}


template <typename Json, typename Adapt, typename CharT>
inline std::streamsize json_parser<Json, Adapt, CharT>::write(char_type const * s, std::streamsize n)
{
    if (!impl_->is_in_progress)
        return EOF;

    std::streamsize i;
    for(i = 0; impl_->is_in_progress && i < n; ++i, ++s) {
        while(impl_->is_in_progress && !impl_->parse_1(*s));
    }
    return i;
}

} }
