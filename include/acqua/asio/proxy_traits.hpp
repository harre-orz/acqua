/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/asio/server_traits.hpp>
#include <memory>

namespace acqua { namespace asio {

template <typename T, typename LowestLayerType = typename T::lowest_layer_type>
struct proxy_traits
    : server_traits<T, LowestLayerType>
{
    static LowestLayerType & socket(std::shared_ptr<T> & soc)
    {
        return soc->server_socket();
    }
};

} }
