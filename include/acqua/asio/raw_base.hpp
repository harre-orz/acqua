#pragma once

#include <acqua/asio/detail/membership.hpp>

namespace acqua { namespace asio { namespace raw_base {

using add_promisc = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_PROMISC>;
using del_promisc = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_PROMISC>;
using add_multicast = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_MULTICAST>;
using del_multicast = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_MULTICAST>;
using add_allmulti = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_ALLMULTI>;
using del_allmulti = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_ALLMULTI>;

} } }
