#pragma once

#include <chrono>
#include <boost/asio/steady_timer.hpp>
#include <acqua/asio/basic_pinger.hpp>
#include <acqua/asio/detail/pinger_service.hpp>
#include <acqua/asio/detail/pinger_impl_v4.hpp>

namespace acqua { namespace asio {

using pinger_v4 = basic_pinger<detail::pinger_service<detail::pinger_impl_v4, boost::asio::steady_timer> >;

} }
