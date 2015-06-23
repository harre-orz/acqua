#include <boost/asio/detail/socket_option.hpp>

namespace acqua { namespace asio { namespace socket_base {

using transparent_v4 = boost::asio::detail::socket_option::boolean<IPPROTO_IP, IP_TRANSPARENT>;
using transparent_v6 = boost::asio::detail::socket_option::boolean<IPPROTO_IPV6, IP_TRANSPARENT>;

} } }
