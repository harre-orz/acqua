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
class md5_context;
}  // cryptographic

using md5_filter = basic_hash_filter<cryptographic::md5_context, 16>; // 16 = MD5_DIGEST_LENGTH

} }

#include <acqua/iostreams/cryptographic/md5_filter.ipp>
