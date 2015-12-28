#pragma once

#include <acqua/email/address.hpp>
#include <acqua/email/headers.hpp>
#include <acqua/email/message.hpp>
#include <acqua/container/recursive_iterator.hpp>

namespace acqua { namespace email {

template <typename String>
class basic_email
{
public:
    using value_type = String;
    using char_type = typename String::value_type;
    using traits_type = typename String::traits_type;
    using address_type = basic_address<value_type>;
    using headers_type = basic_headers<value_type>;
    using message_type = basic_message<value_type>;
    using istream_type = std::basic_istream<char_type, traits_type>;
    using ostream_type = std::basic_ostream<char_type, traits_type>;
    using iterator = acqua::container::preordered_recursive_iterator<
        message_type, typename message_type::iterator>;
    using const_iterator = acqua::container::preordered_recursive_iterator<
        message_type const, typename message_type::const_iterator>;

public:
    basic_email();

    /*!
      デバッグダンプ
     */
    void dump(std::ostream & os)
    {
        for(auto it = begin(), e = end(); it != e; ++it) {
            it->headers.dump(os);
            os << it->str() << std::endl;
        }
    }

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    //! ヘッダー取得.
    typename headers_type::disposition & operator[](value_type const & key)
    {
        return impl_->headers[key];
    }

    //! メッセージ取得
    message_type & operator*()
    {
        return *impl_;
    }

    //! メッセージ取得
    message_type const & operator*() const
    {
        return *impl_;
    }

    //! メッセージ取得
    message_type * operator->()
    {
        return &*impl_;
    }

    //! メッセージ取得
    message_type const * operator->() const
    {
        return &*impl_;
    }

    /*!
      入力ストリーム is を終端までを message_type として取り込む.
      email がマルチパートのときは、最下にあるメッセージの次に位置に is が取り込まれる
      email がマルチパート出ない場合は、マルチパート化されて、最後の位置に is が取り込まれる
      具体的には、今まで使用していたメッセージが一段下に下がり、最上位にマルチパートが追加される
      そして、今まで使用していたメッセージの次の位置に is が追加される
    */
    bool add_attachment(istream_type & is, value_type const & contenttype = value_type());

private:
    std::unique_ptr<message_type> impl_;
};

using email = basic_email<std::string>;
using wemail = basic_email<std::wstring>;

} }

#include <acqua/email/impl/email.ipp>
