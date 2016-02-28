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

struct hmac_md5_engine;
struct hmac_sha1_engine;
struct hmac_sha256_engine;
struct hmac_sha512_engine;

template <typename Engine>
class hmac_context;

using hmac_md5_filter = basic_hash_filter<hmac_context<hmac_md5_engine>, 16>;
using hmac_sha1_filter = basic_hash_filter<hmac_context<hmac_sha1_engine>, 20>;
using hmac_sha256_filter = basic_hash_filter<hmac_context<hmac_sha256_engine>, 32>;
using hmac_sha512_filter = basic_hash_filter<hmac_context<hmac_sha512_engine>, 64>;

} // crypto

using hmac_md5_filter = crypto::hmac_md5_filter;
using hmac_sha1_filter = crypto::hmac_sha1_filter;
using hmac_sha256_filter = crypto::hmac_sha256_filter;
using hmac_sha512_filter = crypto::hmac_sha512_filter;

} }

#include <acqua/iostreams/crypto/hmac_filter.ipp>
