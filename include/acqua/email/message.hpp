/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <deque>
#include <boost/system/error_code.hpp>
#include <acqua/email/headers.hpp>
#include <acqua/container/recursive_iterator.hpp>

namespace acqua { namespace email {

template <typename String>
class basic_message
{
public:
    using value_type = String;
    using char_type = typename value_type::value_type;
    using traits_type = typename value_type::traits_type;
    using streambuf_type = std::basic_streambuf<char_type, traits_type>;
    using headers_type = basic_headers<value_type>;

private:
    using allocator_type = typename value_type::allocator_type;
    using stringbuf_type = std::basic_stringbuf<char_type, traits_type, allocator_type>;
    using filebuf_type = std::basic_filebuf<char_type, traits_type>;
    using subpart_type = std::deque<basic_message>;
    using subpart_iterator = typename subpart_type::iterator;
    using const_subpart_iterator = typename subpart_type::const_iterator;

public:
    static subpart_iterator begin(basic_message & mes)
    {
        return mes.subpart_.begin();
    }

    static const_subpart_iterator begin(basic_message const & mes)
    {
        return mes.subpart_.begin();
    }

    static subpart_iterator end(basic_message & mes)
    {
        return mes.subpart_.end();
    }

    static const_subpart_iterator end(basic_message const & mes)
    {
        return mes.subpart_.end();
    }

    using iterator = acqua::container::preordered_recursive_iterator<
        basic_message, subpart_iterator, &basic_message::begin, &basic_message::end>;

    using const_iterator = acqua::container::preordered_recursive_iterator<
        basic_message const, const_subpart_iterator, &basic_message::begin, &basic_message::end>;

public:
    // *** ペイロード *** //

    /*!
      set_payload() で作成されたペイロードバッファを返す.

      ペイロードバッファが作成されていないと nullptr を返す。
     */
    streambuf_type * get_payload()
    {
        return streambuf_.get();
    }

     /*!
      メッセージ形式で、ペイロードバッファを作成する.
     */
    streambuf_type * set_payload();

    /*!
      ファイル形式で、ペイロードバッファを作成する.

      ファイルが存在しない場合は新規作成する。
      ファイルが存在する場合 overwrite が false のときは追記モード、true のときは書き換えモードになる。
      どちらの場合でも、ファイルのオープンに失敗した場合は 例外 boost::system::system_error が発生する。
    */
    streambuf_type * set_payload(std::string const & filename, boost::system::error_code & ec, bool overwrite = false);

    streambuf_type * set_payload(std::string const & filename, bool overwrite = false);

    /*!
      set_payload() で作成されたペイロードバッファを返す.

      ペイロードバッファが作成されていないと、メッセージ形式のペイロードバッファを作成する
     */
    operator streambuf_type *();

    /*!
      ペイロードバッファを文字列にして返す.

      バッファから文字列に全文をコピーするため、非常に遅い
    */
    value_type str() const;

    /*!
      ペイロードバッファを filename に保存する.
     */
    bool save_as(std::string const & filename, boost::system::error_code & ec);

    bool save_as(std::string const & filename);

public:
    // *** サブパート *** //

    bool has_subpart() const
    {
        return !subpart_.empty();
    }

    /*!
      サブパートを作成する.
    */
    basic_message & add_subpart()
    {
        subpart_.emplace_back();
        return subpart_.back();
    }

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

public:
    // *** ヘッダー *** //
    headers_type headers;

private:
    std::unique_ptr<streambuf_type> streambuf_;
    subpart_type subpart_;
};

using message = basic_message<std::string>;
using wmessage = basic_message<std::wstring>;

} }

#include <acqua/email/impl/message.ipp>