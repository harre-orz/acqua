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

template <typename Content>
class non_encoded_post
{
public:
    explicit non_encoded_post(Content content)
        : content_(content) {}

    void method(std::ostream & os) const
    {
        os << "POST";
    }

    void content(std::ostream & os) const
    {
        write_content(os, content_);
    }

private:
    template <typename Map, typename Map::mapped_type * = nullptr>
    void write_content(std::ostream & os, Map const & map) const
    {
        boost::asio::streambuf buf;
        std::ostream oss(&buf);

        auto it = map.begin();
        if (it != map.end()) {
            oss << it->first << '=' << it->second;
            while(++it != map.end()) {
                oss << '&' << it->first << '=' << it->second;
            }
        }

        os <<
            "Content-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: " << buf.size() << "\r\n"
            "\r\n" << &buf;
    }

private:
    Content content_;
};

}

template <typename Client, typename Content>
inline typename Client::result_ptr wpost(Client & client, std::string const & url, Content content)
{
    return wpost(client, uri(url), content);
}

template <typename Client, typename Uri, typename Content, typename std::enable_if<std::is_base_of<detail::uri_base, Uri>::value>::type * = nullptr>
inline typename Client::result_ptr wpost(Client & client, Uri const & uri, Content content)
{
    return connect(client, detail::non_encoded_post<Content>(content), uri)->start();
}


template <typename Client, typename Content, typename Handler>
inline void wpost(Client & client, std::string const & url, Content content, Handler handler)
{
    wpost(client, uri(url), content, handler);
}

template <typename Client, typename Uri, typename Content, typename Handler, typename std::enable_if<std::is_base_of<detail::uri_base, Uri>::value>::type * = nullptr>
inline void wpost(Client & client, Uri const & uri, Content content, Handler handler)
{
    connect(client, detail::non_encoded_post<Content>(content), uri, handler);
}

} }
