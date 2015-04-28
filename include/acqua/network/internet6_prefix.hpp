#pragma once

#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/prefix_address.hpp>

namespace acqua { namespace network {

using internet6_prefix = detail::prefix_address<internet6_address>;

} }
