/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/iostreams/json_adapt.hpp>
#include <boost/iostreams/categories.hpp>
#include <memory>

namespace acqua { namespace iostreams {

template <
    typename Json,
    typename Adapt = json_adapt<Json>,
    typename CharT = typename Adapt::char_type
    >
class json_parser
{
    struct impl;

public:
    using char_type = CharT;
    using category = boost::iostreams::sink_tag;

public:
    json_parser(Json & json);

    std::streamsize write(char_type const * s, std::streamsize n);

    explicit operator bool() const noexcept;

private:
    std::shared_ptr<impl> impl_;
};

} }

#include <acqua/iostreams/impl/json_parser.ipp>
