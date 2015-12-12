/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <boost/operators.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace acqua { namespace email {

/*!
  メールアドレスクラス.

  メンバ変数 namespec と addrspec を直接設定するのではなく、make_address もしくは parse_to_addresses からの設定を推奨
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
  メールアドレス文字列から address クラスを作成する.

  メールアドレス文字列の例
  - " example.com " => namespec = "", addrspec = "example.com"
  - " <example.com> " => namespec = "", addrspec = "example.com"
  - " foo bar < example.com > " => namespec = "foo bar", addrspec = "example.com"
 */
template <typename String, typename It>
basic_address<String> make_address(It beg, It end, boost::system::error_code & ec);

template <typename String>
inline basic_address<String> make_address(String const & str, boost::system::error_code & ec)
{
    return make_address<String>(str.begin(), str.end(), ec);
}

template <typename String>
inline basic_address<String> make_address(String const & str)
{
    boost::system::error_code ec;
    return make_address<String>(str.begin(), str.end(), ec);
    if (ec) throw boost::system::system_error(ec, "make_address");
}

inline address make_address(char const * str, boost::system::error_code & ec)
{
    return make_address<std::string>(str, str + std::char_traits<char>::length(str), ec);
}

inline address make_address(char const * str)
{
    boost::system::error_code ec;
    return make_address<std::string>(str, str + std::char_traits<char>::length(str), ec);
    if (ec) throw boost::system::system_error(ec, "make_address");
}

inline waddress make_address(wchar_t const * str, boost::system::error_code & ec)
{
    return make_address<std::wstring>(str, str + std::char_traits<wchar_t>::length(str), ec);
}

inline waddress make_address(wchar_t const * str)
{
    boost::system::error_code ec;
    return make_address<std::wstring>(str, str + std::char_traits<wchar_t>::length(str), ec);
    if (ec) throw boost::system::system_error(ec, "make_address");
}

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
