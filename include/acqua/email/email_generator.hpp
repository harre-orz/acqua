#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <boost/iostreams/categories.hpp>
#include <memory>

namespace acqua { namespace email {

template <typename String>
class basic_email;

template <typename String>
class basic_email_generator
{
    struct impl;

public:
    using char_type = char;
    using category = boost::iostreams::source_tag;

    explicit basic_email_generator(basic_email<String> & email);

    explicit operator bool() const;

    std::streamsize read(char_type * s, std::streamsize n);

private:
    std::shared_ptr<impl> impl_;
};

using email_generator = basic_email_generator<std::string>;
using wemaii_generator = basic_email_generator<std::wstring>;

} }

#include <acqua/email/email_generator.ipp>
