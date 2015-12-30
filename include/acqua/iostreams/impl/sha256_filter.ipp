#pragma once

extern "C" {
#include <openssl/sha.h>
}

#include <cstring>
#include <acqua/iostreams/sha256_filter.hpp>

namespace acqua { namespace iostreams {

struct sha256_context::impl
{
    explicit impl(unsigned char * buffer_)
        : buffer(buffer_) {}
    SHA256_CTX context;
    unsigned char * buffer;
};


inline sha256_context::sha256_context(unsigned char * buffer)
    : impl_(new impl(buffer)) {}

inline void sha256_context::init(boost::system::error_code & ec)
{

    std::memset(impl_->buffer, 0, buffer_size);
    if (SHA256_Init(&impl_->context) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}

inline void sha256_context::update(char const * s, std::size_t n, boost::system::error_code & ec)
{
    if (SHA256_Update(&impl_->context, s, n) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}

inline void sha256_context::finish(boost::system::error_code & ec)
{
    if (SHA256_Final(impl_->buffer, &impl_->context) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}


} }
