/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/prefix_address.hpp>

namespace acqua { namespace network {

using internet4_prefix = detail::prefix_address<internet4_address>;

} }
