/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/website/detail/connect.hpp>
#include <acqua/website/detail/content_buffer.hpp>

namespace acqua { namespace website {

template <typename Client>
inline typename Client::result_ptr wget(Client & client, std::string const & uri)
{
    return detail::connect(client, "GET", uri, detail::no_content_buffer())->start();
}

template <typename Client, typename Handler>
inline void wget(Client & client, std::string const & uri, Handler handler)
{
    detail::connect(client, "GET", uri, detail::no_content_buffer())->async_start(handler);
}

} }
