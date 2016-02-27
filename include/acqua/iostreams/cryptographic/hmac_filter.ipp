#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/iostreams/cryptographic/hmac_filter.hpp>
#include <acqua/iostreams/error.hpp>

extern "C" {
#include <openssl/hmac.h>
}

namespace acqua { namespace iostreams { namespace cryptographic {

struct hmac_md5_engine
{
    ::EVP_MD const * evp() const { return EVP_md5(); }
    static const error::cryptographic_errors errc = error::hmac_md5_error;
};

struct hmac_sha1_engine
{
    ::EVP_MD const * evp() const { return EVP_sha1(); }
    static const error::cryptographic_errors errc = error::hmac_sha1_error;
};

struct hmac_sha256_engine
{
    ::EVP_MD const * evp() const { return EVP_sha256(); }
    static const error::cryptographic_errors errc = error::hmac_sha256_error;
};

struct hmac_sha512_engine
{
    ::EVP_MD const * evp() const { return EVP_sha512(); }
    static const error::cryptographic_errors errc = error::hmac_sha512_error;
};

template <typename Engine>
class hmac_context
    : private Engine
{
public:
    hmac_context() noexcept
    {
        HMAC_CTX_init(&context);
    }

    ~hmac_context() noexcept
    {
        HMAC_CTX_cleanup(&context);
    }

    void init(void const * key, std::size_t len, boost::system::error_code & ec) noexcept
    {
        if (::HMAC_Init_ex(&context, key, static_cast<int>(len), Engine::evp(), NULL) != 1)
            ec = make_error_code(Engine::errc);
    }

    void update(char const * s, std::size_t n, boost::system::error_code & ec) noexcept
    {
        if (::HMAC_Update(&context, reinterpret_cast<unsigned char const *>(s), n) != 1)
            ec = make_error_code(Engine::errc);
    }

    void finish(unsigned char * buffer, std::size_t size, boost::system::error_code & ec) noexcept
    {
        unsigned int len = static_cast<unsigned int>(size);
        if (::HMAC_Final(&context, buffer, &len) != 1)
            ec = make_error_code(Engine::errc);
    }

private:
    ::HMAC_CTX context;
};

} } }
