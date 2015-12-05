#pragma once

extern "C" {
#include <linux/rtnetlink.h>
}

#include <type_traits>

namespace acqua { namespace asio { namespace netlink {

struct link_tag
{
    static const int group = RTMGRP_LINK;
};

struct stats_tag
{
    static const int group = RTMGRP_LINK;
};

struct notify_tag
{
    static const int group = RTMGRP_NOTIFY;
};

struct neighbor_v4_tag
{
    static const int group = RTMGRP_NEIGH;
};

struct neighbor_v6_tag
{
    static const int group = RTMGRP_NEIGH;
};

struct neighbor_tag
    : neighbor_v4_tag, neighbor_v6_tag
{
};

struct ifaddr_v4_tag
{
    static const int group = RTMGRP_IPV4_IFADDR;
};

struct ifaddr_v6_tag
{
    static const int group = RTMGRP_IPV6_IFADDR;
};

struct ifaddr_tag
    : ifaddr_v4_tag, ifaddr_v6_tag
{
};

} } }
