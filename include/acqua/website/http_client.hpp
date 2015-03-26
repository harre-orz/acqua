/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/steady_timer.hpp>
#include <acqua/website/client_result.hpp>
#include <acqua/website/basic_http_client.hpp>

namespace acqua { namespace website {

using http_client = basic_http_client<client_result, boost::asio::steady_timer>;

} }
