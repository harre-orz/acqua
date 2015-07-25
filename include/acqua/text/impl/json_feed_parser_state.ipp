#pragma once

#include <limits>
#include <boost/locale.hpp>
#include <boost/variant.hpp>

namespace acqua { namespace text { namespace impl {

enum literal { l_null, l_true, l_false };

class any_parser
{
public:
    template <typename CharT, typename State>
    bool operator()(CharT const & ch, State & state)
    {
        switch(ch) {
            case '\0':
                return true;
            case ' ': case '\t': case '\r': case '\n':
                return true;
            case 'n': case 'N':
                state.next_null();
                return true;
            case 't': case 'T':
                state.next_boolean(true);
                return true;
            case 'f': case 'F':
                state.next_boolean(false);
                return true;
            case '"':
                state.next_string();
                return true;
            case '[':
                state.next_array();
                return true;
            case '{':
                state.next_object();
                return true;
            // case '-': case '+':
            // case '0' ... '9':
            default:
                state.next_number(ch);
                return true;
        }

        state.failure();
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

    template <typename CharT, typename State>
    bool operator()(CharT const & ch, State & state)
    {
        if (*++lit_ == '\0') {
            switch(l_) {
                case l_null:
                    state.next_terminated(nullptr);
                    break;
                case l_true:
                    state.next_terminated(true);
                    break;
                case l_false:
                    state.next_terminated(false);
                    break;
            }
            return false;
        } else if (std::tolower(ch, std::locale::classic()) == *lit_) {
            return true;
        }

        state.failure();
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

    template <typename CharT, typename State>
    bool operator()(CharT const & ch, State & state)
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
                state.next_terminated(decimal_);
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
                state.next_terminated((double)decimal_ / (double)fraction_);
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
                state.next_terminated((double)decimal_ * std::pow(10, exponent_) / (double)fraction_);
                return false;
        }

        state.failure();
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
    template <typename CharT, typename State>
    bool operator()(CharT const & ch, State & state)
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
                        state.failure();
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
            state.next_terminated(std::move(str_));
            return true;
        } else if (ch == '\\') {
            escape_ = true;
            hex_[0] = 0;
            return true;
        } else if (std::isprint(ch, std::locale::classic())) {
            str_ += ch;
            return true;
        }

        state.failure();
        return true;
    }
};


template <typename State>
class array_parser
{
    std::unique_ptr<State> data_;
    int index_ = 0;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, State & state)
    {
        if (data_ && !data_->is_terminated()) {
            return data_->do_parse_1(ch);
        } else if (ch == ']') {
            state.next_terminated();
            return true;
        } else if (!data_) {
            state.next_child(index_++, data_);
            return data_->do_parse_1(ch);
        } else if (ch == ',') {
            state.next_child(index_++, data_);
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        state.failure();
        return true;
    }
};


template <typename State, typename String>
class object_parser
{
    struct string : string_parser<String>
    {
        explicit string(State & parent) : parent_(parent) {}
        bool is_terminated() const { return is_terminated_; }
        template <typename T>
        void next_terminated(T const &) { is_terminated_ = true; }
        void failure() { parent_.failure(); }
        State & parent_;
        bool is_terminated_ = false;
        using string_parser<String>::str_;
    };
    std::unique_ptr<string> key_;
    std::unique_ptr<State> val_;
    bool first_ = true;

public:
    template <typename CharT>
    bool operator()(CharT const & ch, State & state)
    {
        if (val_) {
            if (!val_->is_terminated())
                return val_->do_parse_1(ch);
            if (ch == '}') {
                state.next_terminated();
                return true;
            }
            if (ch == ',') {
                key_.release();
                val_.release();
                return true;
            }
        } else if (!key_) {
            if (first_ && ch == '}') {
                state.next_terminated();
                return true;
            }
            if (ch == '"') {
                first_ = false;
                key_.reset(new string(state));
                return true;
            }
        } else if (!key_->is_terminated()) {
            return (*key_)(ch, *key_);
        } else if (ch == ':') {
            state.next_child(std::move(key_->str_), val_);
            return true;
        }
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;
        state.failure();
        return true;
    }
};


template <typename CharT, typename Json, typename Adapt>
class json_feed_parser_state
    : public boost::variant<any_parser, literal_parser, number_parser,
                            string_parser<std::basic_string<CharT> >,
                            array_parser<json_feed_parser_state<CharT, Json, Adapt> >,
                            object_parser<json_feed_parser_state<CharT, Json, Adapt>, std::basic_string<CharT> > >
    , Adapt
{
private:
    using any_type = any_parser;
    using literal_type = literal_parser;
    using number_type = number_parser;
    using string_type = string_parser< std::basic_string<CharT> >;
    using array_type = array_parser< json_feed_parser_state<CharT, Json, Adapt> >;
    using object_type = object_parser< json_feed_parser_state<CharT, Json, Adapt>, std::basic_string<CharT> >;
    using base_type = boost::variant<any_type, literal_type, number_type, string_type, array_type, object_type>;

    struct feed_visitor : boost::static_visitor<bool>
    {
        CharT const & ch_;
        json_feed_parser_state & state_;

        feed_visitor(CharT const & ch, json_feed_parser_state & state)
            : ch_(ch), state_(state) {}

        template <typename Parser>
        bool operator()(Parser & parser) const
        {
            return parser(ch_, state_);
        }
    };

public:
    json_feed_parser_state(boost::system::error_code & error, Json & json)
        : Adapt(json), error_(error) {}

    bool is_terminated() const
    {
        return is_terminated_;
    }

    bool do_parse_1(CharT ch)
    {
        return boost::apply_visitor(feed_visitor(ch, *this), *this);
    }

    void next_null()
    {
        *static_cast<base_type *>(this) = literal_type(l_null);
    }

    void next_boolean(bool val)
    {
        *static_cast<base_type *>(this) = literal_type(val ? l_true : l_false);
    }

    void next_number(char ch)
    {
        *static_cast<base_type *>(this)
            = (ch == '-') ? number_type(0, true)
            : (ch == '+') ? number_type(0, false)
            :               number_type(ch - '0', false);
    }

    void next_string()
    {
        *static_cast<base_type *>(this) = string_type();
    }

    void next_array()
    {
        *static_cast<base_type *>(this) = array_type();
    }

    void next_object()
    {
        *static_cast<base_type *>(this) = object_type();
    }

    template <typename T>
    void next_child(T const & t, std::unique_ptr<json_feed_parser_state> & state)
    {
        state.reset(new json_feed_parser_state(error_, static_cast<Adapt *>(this)->add_child(t)));
    }

    template <typename T>
    void next_child(T && t, std::unique_ptr<json_feed_parser_state> & state)
    {
        state.reset(new json_feed_parser_state(error_, static_cast<Adapt *>(this)->add_child(std::move(t))));
    }

    void next_terminated()
    {
        *static_cast<base_type *>(this) = any_type();
        is_terminated_ = true;
    }

    template <typename T>
    void next_terminated(T const & t)
    {
        static_cast<Adapt *>(this)->data(t);
        next_terminated();
    }

    template <typename T>
    void next_terminated(T && t)
    {
        static_cast<Adapt *>(this)->data(std::move(t));
        next_terminated();
    }

    void failure()
    {
        error_.assign(EIO, boost::system::generic_category());
    }

private:
    boost::system::error_code & error_;
    bool is_terminated_ = false;
};

} } }
