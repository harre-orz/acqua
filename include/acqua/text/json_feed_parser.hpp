/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>
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

        next.failure();
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
                    next.complete(nullptr);
                    break;
                case l_true:
                    next.complete(true);
                    break;
                case l_false:
                    next.complete(false);
                    break;
            }
            return false;
        } else if (std::tolower(ch, std::locale::classic()) == *lit_) {
            return true;
        }

        next.failure();
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
                next.complete(decimal_);
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
                next.complete((double)decimal_ / (double)fraction_);
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
                next.complete((double)decimal_ * std::pow(10, exponent_) / (double)fraction_);
                return false;
        }

        next.failure();
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
                        next.failure();
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
            next.complete(str_);
            return true;
        } else if (ch == '\\') {
            escape_ = true;
            hex_[0] = 0;
            return true;
        } else if (std::isprint(ch, std::locale::classic())) {
            str_ += ch;
            return true;
        }

        next.failure();
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
        if (data_ && data_->progress()) {
            return data_->parse(ch);
        } else if (ch == ']') {
            next.complete();
            return true;
        } else if (!data_) {
            data_ = next.child(index_++);
            return data_->parse(ch);
        } else if (ch == ',') {
            data_ = next.child(index_++);
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        next.failure();
        return true;
    }
};


template <typename Parser, typename String>
class object_parser
{
    struct string : string_parser<String>
    {
        explicit string(Parser & parent) : parent_(parent) {}
        bool progress() const { return !compl_; }
        template <typename T>
        void complete(T const &) { compl_ = true; }
        void failure() { parent_.failure(); }
        String const & str() const { return this->str_; }
        Parser & parent_;
        bool compl_ = false;
    };
    std::unique_ptr<string> key_;
    std::unique_ptr<Parser> val_;
    bool first_ = true;

public:
    template <typename CharT>
    bool parse(CharT const & ch, Parser & next)
    {
        if (val_) {
            if (val_->progress())
                return val_->parse(ch);
            if (ch == '}') {
                next.complete();
                return true;
            }
            if (ch == ',') {
                key_.release();
                val_.release();
                return true;
            }
        } else if (!key_) {
            if (first_ && ch == '}') {
                next.complete();
                return true;
            }
            if (ch == '"') {
                first_ = false;
                key_.reset(new string(next));
                return true;
            }
        } else if (key_->progress()) {
            return key_->parse(ch, *key_);
        } else if (ch == ':') {
            val_ = next.child(key_->str());
            return true;
        }

        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
            return true;

        next.failure();
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
    using adaptor_type = parse_adaptor<Destinate>;

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
    feed_parser(Destinate & dest, boost::system::error_code & error)
        : base_type(), adapt_(dest), error_(error) {}

    feed_parser(feed_parser const & rhs)
        : base_type(), adapt_(rhs.dest_), error_(rhs.error_) {}

    //! ch をパースする.
    //! 次の文字へ進むときは true, 同じ文字をもう一度パースするときは false を返す
    bool parse(CharT ch)
    {
        return boost::apply_visitor(parse_visitor(*this, ch), *this);
    }

    //! null のパーサになる
    void null()
    {
        static_cast<base_type &>(*this) = literal_type(l_null);
    }

    //! true/false のパーサになる
    void boolean(bool val)
    {
        static_cast<base_type &>(*this) = literal_type(val ? l_true : l_false);
    }

    //! 数値のパーサになる
    void number(char ch)
    {
        static_cast<base_type &>(*this)
            = (ch == '-') ? number_type(0, true)
            : (ch == '+') ? number_type(0, false)
            :               number_type(ch - '0', false);
    }

    //! 文字列のパーサになる
    void string()
    {
        static_cast<base_type &>(*this) = string_type();
    }

    //! 配列のパーサになる
    void array()
    {
        static_cast<base_type &>(*this) = array_type();
    }

    //! オブジェクトのパーサになる
    void object()
    {
        static_cast<base_type &>(*this) = object_type();
    }

    //! 現在のパースを成功にして、次のパーサを選択するパーサになる
    void complete()
    {
        static_cast<base_type &>(*this) = any_type();
        compl_ = true;
    }

    //! 現在のパースが成功にして値 T を保存し、次のパーサを選択するパーサになる
    template <typename T>
    void complete(T const & t)
    {
        adapt_.parse_value(t);
        complete();
    }

    //! パースに失敗した状態になる
    void failure()
    {
        error_.assign(EIO, boost::system::generic_category());
    }

    //! パースが進行中であれば true を返す
    bool progress() const
    {
        return !compl_;
    }

    //! 現在のパースが成功にして値 T を保存し、入れ子パーサを返す
    template <typename T>
    std::unique_ptr<feed_parser> child(T const & t)
    {
        return std::unique_ptr<feed_parser>(new feed_parser(adapt_.parse_child(t), error_));
    }

    boost::system::error_code const & error() const
    {
        return error_;
    }

private:
    adaptor_type adapt_;
    boost::system::error_code & error_;
    bool compl_ = false;
};

} // json_impl


/*!
  JSONストリーミングパーサ.

  バッファを持たずに状態を保持しながら逐次的にパースする
 */
template <typename CharT, typename Destinate>
class json_feed_parser
{
    using data_type = json_impl::feed_parser<CharT, Destinate>;

public:
    json_feed_parser(Destinate & dest)
        : data_(dest, error_) {}

    //! パースの処理中であれば true を返す
    bool progress() const
    {
        return !data_.error() && data_.progress();
    }

    //! パースが継続できないとき、error に値が入る
    boost::system::error_code const & error() const
    {
        return data_.error();
    }

    //! ch をストリーミングパースする
    json_feed_parser & parse(CharT ch)
    {
        while(!data_.parse(ch));
        return *this;
    }

    //! is がEOFになるか、パースが正常/異常終了するまで、is から文字を取得し続ける
    friend std::basic_istream<CharT> & operator>>(std::basic_istream<CharT> & is, json_feed_parser<CharT, Destinate> & rhs)
    {
        CharT ch;
        while(is.get(ch).good() && rhs.parse(ch).good());
        return is;
    }

private:
    data_type data_;
    boost::system::error_code error_;
};

} }
