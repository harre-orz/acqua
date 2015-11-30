#pragma once

#include <boost/asio/steady_timer.hpp>
#include <acqua/asio/basic_pinger.hpp>
#include <acqua/asio/detail/pinger_service.hpp>
#include <acqua/asio/detail/pinger_impl_v6.hpp>

namespace acqua { namespace asio {

using pinger_v6 = basic_pinger<detail::pinger_service<detail::pinger_impl_v6, boost::asio::steady_timer> >;

} }
