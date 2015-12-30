#pragma once

#include <array>
#include <memory>
#include <acqua/iostreams/basic_hash_filter.hpp>

namespace acqua { namespace iostreams {

class sha256_context
{
    struct impl;

public:
    using buffer_type = std::array<unsigned char, 32>;

    explicit sha256_context(buffer_type & buffer);

    void init(boost::system::error_code & ec);

    void update(char const * s, std::size_t n, boost::system::error_code & ec);

    void finish(boost::system::error_code & ec);

private:
    std::shared_ptr<impl> impl_;
};

using sha256_filter = basic_hash_filter<sha256_context>;

} }

#include <acqua/iostreams/impl/sha256_filter.ipp>
