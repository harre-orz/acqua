#pragma once

extern "C" {
#include <openssl/sha.h>
}

#include <acqua/iostreams/sha256_filter.hpp>

namespace acqua { namespace iostreams {

struct sha256_context::impl
{
    explicit impl(buffer_type & buffer_)
        : buffer(buffer_) {}
    SHA256_CTX context;
    buffer_type &  buffer;
};


inline sha256_context::sha256_context(buffer_type & buffer)
    : impl_(new impl(buffer)) {}

inline void sha256_context::init(boost::system::error_code & ec)
{
    impl_->buffer.fill(0);
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
    if (SHA256_Final(impl_->buffer.begin(), &impl_->context) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}


} }
