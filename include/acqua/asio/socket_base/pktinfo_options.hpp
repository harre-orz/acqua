#pragma once

extern "C" {
#include <sys/socket.h>
}

#include <boost/asio/detail/socket_option.hpp>

namespace acqua { namespace asio { namespace socket_base {

using pktinfo_v4 = boost::asio::detail::socket_option::boolean<IPPROTO_IP, IP_PKTINFO>;
using pktinfo_v6 = boost::asio::detail::socket_option::boolean<IPPROTO_IPV6, IP_PKTINFO>;

} } }

