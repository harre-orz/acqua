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

namespace acqua { namespace webclient {

template <typename Client, typename Req, typename Uri>
inline std::shared_ptr<typename Client::socket_type> connect(Client & client,  Req const & req,  Uri const & uri)
{
    int mode;
    int port = 0;
    std::string host;

    namespace qi = boost::spirit::qi;
    qi::symbols<char, int> sym;
    sym.add("http://", 1)("https://", 2);
    auto it = uri.begin();
    if (qi::parse(it, uri.end(), sym >> *(qi::char_ - ':' - '/') >> -(':' >> qi::int_), mode, host, port)) {
        switch(mode) {
            case 1: {
                if (!port)
                    port = 80;
                auto socket = client.http_connect(host, std::to_string(port));
                std::ostream & os = (*socket);
                req.method(os);
                os << ' ';
                if (it == uri.end() || *it != '/')
                    os << '/';
                std::copy(it, uri.end(), std::ostreambuf_iterator<char>(os));
                uri.query(os);
                os << " "
                    "HTTP/1.1\r\n"
                    "Host: " << host << "\r\n"
                    "Connection: " << (client.enabled_keep_alive() ? "Keep-Alive" : "Close") << "\r\n";
                uri.header(os);
                req.content(os);
                return socket;
            }
            case 2: {
                if (auto ctx = client.get_context()) {
                    if (!port)
                        port = 443;
                    auto socket = client.https_connect(*ctx, host, std::to_string(port));
                    std::ostream & os = (*socket);
                    req.method(os);
                    os << ' ';
                    if (it == uri.end() || *it != '/')
                        os << '/';
                    std::copy(it, uri.end(), std::ostreambuf_iterator<char>(os));
                    uri.query(os);
                    os << " "
                        "HTTP/1.1\r\n"
                        "Host: " << host << "\r\n"
                        "Connection: " << (client.enabled_keep_alive() ? "Keep-Alive" : "Close") << "\r\n";
                    uri.header(os);
                    req.content(os);
                    return socket;
                } else {
                    throw std::runtime_error("unsupported https");
                }
            }
        }
    }

    throw std::runtime_error("invalid argument");
}

} }
