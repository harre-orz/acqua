#pragma once

#include <acqua/asio/server_traits.hpp>

namespace acqua { namespace asio {

template <typename T, typename LowestLayerType = typename T::lowest_layer_type>
struct proxy_traits
    : server_traits<T, LowestLayerType>
{
    static LowestLayerType & socket(std::shared_ptr<T> soc)
    {
        return soc->server_socket();
    }
};

} }
