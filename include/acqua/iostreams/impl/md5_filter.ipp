#pragma once

extern "C" {
#include <openssl/md5.h>
}

#include <cstring>
#include <acqua/iostreams/md5_filter.hpp>

namespace acqua { namespace iostreams {

struct md5_context::impl
{
    explicit impl(unsigned char * buffer_)
        : buffer(buffer_) {}

    MD5_CTX context;
    unsigned char * buffer;
};


inline md5_context::md5_context(unsigned char * buffer)
    : impl_(new impl(buffer)) {}


inline void md5_context::init(boost::system::error_code & ec)
{
    std::memset(impl_->buffer, 0, buffer_size);
    if (MD5_Init(&impl_->context) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}


inline void md5_context::update(char const * s, std::size_t n, boost::system::error_code & ec)
{
    if (MD5_Update(&impl_->context, s, n) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}


inline void md5_context::finish(boost::system::error_code & ec)
{
    if (MD5_Final(impl_->buffer, &impl_->context) != 1)
        ec = make_error_code(boost::system::errc::bad_address);
}

} }
