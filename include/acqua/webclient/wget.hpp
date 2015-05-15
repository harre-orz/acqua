/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <type_traits>
#include <acqua/webclient/uri.hpp>
#include <acqua/webclient/connect.hpp>

namespace acqua { namespace webclient {

namespace detail {

class get
{
public:
    void method(std::ostream & os) const
    {
        os << "GET";
    }

    void content(std::ostream & os) const
    {
        os << "\r\n";
    }
};

}

template <typename Client>
inline typename Client::result_ptr wget(Client & client, std::string const & url)
{
    return wget(client, uri(url));
}

template <typename Client, typename Uri, typename std::enable_if<std::is_base_of<detail::uri_base, Uri>::value>::type * = nullptr>
inline typename Client::result_ptr wget(Client & client, Uri const & uri)
{
    return connect(client, detail::get(), uri)->start();
}


template <typename Client, typename Handler>
inline void wget(Client & client, std::string const & url, Handler handler)
{
    wget(client, uri(url), handler);
}

template <typename Client, typename Uri, typename Handler, typename std::enable_if<std::is_base_of<detail::uri_base, Uri>::value>::type * = nullptr>
inline void wget(Client & client, Uri const & uri, Handler handler)
{
    connect(client, detail::get(), uri)->async_start(handler);
}

} }
