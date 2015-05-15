/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <type_traits>
#include <boost/blank.hpp>

namespace acqua { namespace webclient { namespace detail {

/*!
  URIの基底クラス
 */
class uri_base
{
    /*!
      URLの先頭の位置を返す.
      URLの文字列は 「スキーマ名://ホスト名[:ポート番号][/パス...[?クエリキー=クエリ名&...]]」 になる。
     */
    std::string::const_iterator begin() const;

    /*!
      URLの終端の位置を返す.
      URLの文字列は 「スキーマ名://ホスト名[:ポート番号][/パス...[?クエリキー=クエリ名&...]]」 になる。
    */
    std::string::const_iterator end() const;

    /*!
      特殊なクエリのデータ構造をリクエストパスに加える.
     */
    void query(std::ostream &) const;

    /*!
      特殊なリクエストヘッダーに加える.
     */
    void header(std::ostream &) const;
};


template <typename Uri, typename Query = boost::blank, typename Header = boost::blank>
class non_encoded_uri
    : public uri_base
{
public:
    using const_iterator = typename std::decay<Uri>::type::const_iterator;

    explicit non_encoded_uri(Uri uri, Query query, Header header)
        : uri_(uri), query_(query), header_(header) {}

    const_iterator begin() const
    {
        return uri_.begin();
    }

    const_iterator end() const
    {
        return uri_.end();
    }

    void query(std::ostream & os) const
    {
        query(os, query_);
    }

    void header(std::ostream & os) const
    {
        header(os, header_);
    }

private:
    void query(std::ostream &, boost::blank) const {}
    void header(std::ostream &, boost::blank) const {}

    template <typename Map, typename Map::mapped_type * = nullptr>
    void query(std::ostream & os, Map const & map) const
    {
        auto it = map.begin();
        if (it != map.end()) {
            os << '?';
            os << it->first << '=' << it->second;
            while(++it != map.end()) {
                os << '&';
                os << it->first << '=' << it->second;
            }
        }
    }

    void query(std::ostream & os, char const * str) const
    {
        if (str && *str != '\0') {
            os << '?';
            os << str;
        }
    }

    template <typename Map, typename Map::mapped_type * = nullptr>
    void header(std::ostream & os, Map const & map) const
    {
        for(auto const & e : map) {
            os << e.first << ':' << ' ' << e.second << '\r' << '\n';
        }
    }

private:
    Uri uri_;
    Query query_;
    Header header_;
};

}

inline detail::non_encoded_uri<std::string const &, boost::blank, boost::blank> uri(std::string const & uri)
{
    return detail::non_encoded_uri<std::string const &, boost::blank, boost::blank>(uri, boost::blank(), boost::blank());
}

template <typename Query, typename Header>
inline detail::non_encoded_uri<std::string const &, Query, Header> uri(std::string const & uri, Query query, Header header)
{
    return detail::non_encoded_uri<std::string const &, Query, Header>(uri, query, header);
}

} }
