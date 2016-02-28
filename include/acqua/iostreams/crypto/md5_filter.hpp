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

class md5_context;
using md5_filter = basic_hash_filter<md5_context, 16>; // 16 = MD5_DIGEST_LENGTH

}  // crypto

using md5_filter = crypto::md5_filter;

} }

#include <acqua/iostreams/crypto/md5_filter.ipp>
