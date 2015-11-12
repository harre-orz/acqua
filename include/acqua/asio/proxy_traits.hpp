#pragma once

#include <acqua/asio/server_traits.hpp>

namespace acqua { namespace asio {

template <typename T>
struct proxy_traits
    : server_traits<T>
{
    template <typename Tag>
    static typename T::lowest_layer_type & socket(Tag, std::shared_ptr<T> soc)
    {
        return soc->server_socket();
    }
};

} }
