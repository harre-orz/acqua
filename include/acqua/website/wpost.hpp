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

template <typename Client, typename ContentBuffer>
inline typename Client::result_ptr wpost(Client & client, std::string const & uri, ContentBuffer content)
{
    return detail::connect(client, "POST", uri, content)->start();
}


template <typename Client, typename ContentBuffer, typename Handler>
inline void wpost(Client & http, std::string const & uri, ContentBuffer content, Handler handler)
{
    detail::connect(client, "POST", uri, content)->async_start(handler);
}

} }
