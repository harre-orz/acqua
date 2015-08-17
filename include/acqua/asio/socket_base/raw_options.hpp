#pragma once

#include <acqua/asio/detail/membership.hpp>

namespace acqua { namespace asio { namespace socket_base {

using acqua::asio::detail::membership;
using add_promisc = membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_PROMISC>;
using del_promisc = membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_PROMISC>;
using add_multicast = membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_MULTICAST>;
using del_multicast = membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_MULTICAST>;
using add_allmulti = membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_ALLMULTI>;
using del_allmulti = membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_ALLMULTI>;

} } }
