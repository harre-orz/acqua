#pragma once

#include <boost/asio/steady_timer.hpp>
#include <acqua/website/client_result.hpp>
#include <acqua/website/basic_http_client.hpp>

namespace acqua { namespace website {

using http_client = basic_http_client<client_result_future, boost::asio::steady_timer>;

} }
