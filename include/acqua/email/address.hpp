/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/system/error_code.hpp>
#include <boost/operators.hpp>
#include <string>

namespace acqua { namespace email {

/*!
  メールアドレスクラス.
 */
template <typename String>
class basic_address
    : private boost::totally_ordered< basic_address<String> >
{
    using char_type = typename String::value_type;
    using traits_type = typename String::traits_type;
    using ostream_type = std::basic_ostream<char_type, traits_type>;

public:
    using value_type = String;

    basic_address() = default;

    basic_address(value_type const & addr)
        : namespec(), addrspec(addr) {}

    basic_address(value_type const & name, value_type const & addr)
        : namespec(name), addrspec(addr) {}

    /*!
      addrspec が メールアドレスになっているか.
    */
    bool is_valid() const;

     /*!
      メールアドレス文字列から address クラスを作成する.

      メールアドレス文字列の例
      - " hoge@example.com " => namespec = "", addrspec = "hoge@example.com"
      - " <hoge@example.com> " => namespec = "", addrspec = "hoge@example.com"
      - " foo bar < hoge@example.com > " => namespec = "foo bar", addrspec = "hoge@example.com"
    */
    static basic_address from_string(String const & str);

    /*!
      メールアドレス文字列から address クラスを作成する.

      メールアドレス文字列の例
      - " hoge@example.com " => namespec = "", addrspec = "hoge@example.com"
      - " <hoge@example.com> " => namespec = "", addrspec = "hoge@example.com"
      - " foo bar < hoge@example.com > " => namespec = "foo bar", addrspec = "hoge@example.com"
    */
    static basic_address from_string(String const & str, boost::system::error_code & ec);

    friend bool operator==(basic_address const & lhs, basic_address const & rhs) noexcept
    {
        return !(lhs.addrspec != rhs.addrspec || lhs.namespec != rhs.namespec);
    }

    friend bool operator<(basic_address const & lhs, basic_address const & rhs) noexcept
    {
        return lhs.namespec < rhs.namespec || lhs.addrspec < rhs.addrspec;
    }

    friend ostream_type & operator<<(ostream_type & os, basic_address const & rhs)
    {
        if (rhs.namespec.empty())
            os << rhs.addrspec;
        else
            os << rhs.namespec << ' ' << '<' << rhs.addrspec << '>';
        return os;
    }

public:
    value_type namespec;
    value_type addrspec;
};

using address = basic_address<std::string>;
using waddress = basic_address<std::wstring>;

/*!
  メールアドレス文字列のカンマ区切りのリスト(メールヘッダーの From, To, Cc で使用される形式)から address クラスのリストを作成する.

  @return 変換に成功した個数を返す
  @param addrs 本関数内で初期化されない
  @tparam Addresses push_back() もしくは emplace_back() が定義されている構造体であること
 */
template <typename It, typename Addresses>
std::size_t parse_to_addresses(It beg, It end, Addresses & addrs);

} }

#include <acqua/email/impl/address.ipp>
