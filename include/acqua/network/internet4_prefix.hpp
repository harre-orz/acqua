/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/basic_prefix_address.hpp>

namespace acqua { namespace network {

using internet4_prefix = basic_prefix_address<internet4_address>;

} }
