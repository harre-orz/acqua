/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/iostreams/basic_hash_filter.hpp>
#include <memory>
#include <array>

namespace acqua { namespace iostreams {

class sha256_context
{
    struct impl;

public:
    static constexpr std::size_t buffer_size = 32;

    explicit sha256_context(unsigned char * buffer);

    void init(boost::system::error_code & ec);

    void update(char const * s, std::size_t n, boost::system::error_code & ec);

    void finish(boost::system::error_code & ec);

private:
    std::shared_ptr<impl> impl_;
};

using sha256_filter = basic_hash_filter<sha256_context>;

} }

#include <acqua/iostreams/impl/sha256_filter.ipp>
