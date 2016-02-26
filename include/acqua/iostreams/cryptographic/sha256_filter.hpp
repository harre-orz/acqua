#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/basic_hash_filter.hpp>

namespace acqua { namespace iostreams {

namespace cryptographic {
class sha256_context;
} // cryptographic

using sha256_filter = basic_hash_filter<cryptographic::sha256_context, 32>;

} }

#include <acqua/iostreams/cryptographic/sha256_filter.ipp>
