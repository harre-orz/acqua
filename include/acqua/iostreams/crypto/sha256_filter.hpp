#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/crypto/basic_hash_filter.hpp>

namespace acqua { namespace iostreams {

namespace crypto {

class sha256_context;
using sha256_filter = basic_hash_filter<sha256_context, 32>;

} // crypto

using sha256_filter = crypto::sha256_filter;

} }

#include <acqua/iostreams/crypto/sha256_filter.ipp>
