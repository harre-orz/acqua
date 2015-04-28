#pragma once

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/prefix_address.hpp>

namespace acqua { namespace network {

using internet4_prefix = detail::prefix_address<internet4_address>;

} }
