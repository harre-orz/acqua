#pragma once

#include <array>
#include <memory>
#include <acqua/iostreams/basic_hash_filter.hpp>

namespace acqua { namespace iostreams {

class md5_context
{
    struct impl;

public:
    static constexpr std::size_t buffer_size = 16;

    explicit md5_context(unsigned char * buffer);

    void init(boost::system::error_code & ec);

    void update(char const * s, std::size_t n, boost::system::error_code & ec);

    void finish(boost::system::error_code & ec);

private:
    std::shared_ptr<impl> impl_;
};

using md5_filter = basic_hash_filter<md5_context>;

} }

#include <acqua/iostreams/impl/md5_filter.ipp>
