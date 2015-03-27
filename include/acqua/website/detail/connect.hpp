/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <string>
#include <cstdlib>
#include <boost/spirit/include/qi.hpp>
#include <acqua/website/detail/content_buffer.hpp>

namespace acqua { namespace website { namespace detail {

template <typename Client, typename Uri, typename ContentBuffer>
inline std::shared_ptr<typename Client::socket_type> connect(Client & client, char const * method, Uri const & uri, ContentBuffer content)
{
    int mode;
    int port;
    std::string host;

    namespace qi = boost::spirit::qi;
    qi::symbols<char, int> sym;
    sym.add("http://", 1)("https://", 2);
    auto it = uri.begin();
    if (qi::parse(it, uri.end(), sym >> *(qi::char_ - ':' - '/') >> ((':' >> qi::int_) | qi::attr(0)), mode, host, port)) {
        switch(mode) {
            case 1: {
                if (!port)
                    port = 80;
                auto socket = client.http_connect(host, std::to_string(port));
                std::ostream & os = (*socket);
                os << method << ' ';
                if (it == uri.end() || *it != '/')
                    os << '/';
                std::copy(it, uri.end(), std::ostreambuf_iterator<char>(os));
                os << " "
                    "HTTP/1.1\r\n"
                    "Host: " << host << "\r\n"
                    "Connection: Keep-Alive\r\n"
                   << content;
                return socket;
            }
            case 2: {
            }
        }
    }

    throw std::runtime_error("invalid argument");
}

} } }
