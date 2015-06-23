#pragma once

#include <acqua/asio/server_traits.hpp>

namespace acqua { namespace asio {

template <typename T>
class proxy_traits
    : public server_traits<T>
{
public:
    template <typename Socket>
    Socket & socket(T * t)
    {
        return t->server_socket();
    }

    template <typename Socket>
    Socket & socket_v4(T * t)
    {
        return socket<Socket>(t);
    }

    template <typename Socket>
    Socket & socket_v6(T * t)
    {
        return socket<Socket>(t);
    }
};

} }
