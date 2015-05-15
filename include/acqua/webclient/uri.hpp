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

class uri_base {};

template <typename Uri, typename Query = boost::blank, typename Header = boost::blank>
class non_encoded_uri
    : public uri_base
{
public:
    using const_iterator = typename std::decay<Uri>::type::const_iterator;

    explicit non_encoded_uri(Uri uri)
        : uri_(uri) {}

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

private:
    Uri uri_;
    Query query_;
    Header header_;
};

}

inline detail::non_encoded_uri<std::string const &> uri(std::string const & uri)
{
    return detail::non_encoded_uri<std::string const &>(uri);
}


} }
