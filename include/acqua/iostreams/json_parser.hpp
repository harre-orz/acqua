/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <memory>
#include <acqua/iostreams/json_adapt.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/system/error_code.hpp>

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

private:
    std::shared_ptr<impl> impl_;
    boost::system::error_code error_ = {};
};

} }

#include <acqua/iostreams/impl/json_parser.ipp>
