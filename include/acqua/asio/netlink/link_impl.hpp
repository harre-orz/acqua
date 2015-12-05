#pragma once

extern "C" {
#include <linux/if_link.h>
}

#include <acqua/asio/netlink/categories.hpp>
#include <acqua/network/linklayer_address.hpp>

namespace acqua { namespace asio { namespace netlink {

template <typename Derived>
class link_impl
{
protected:
    ~link_impl() = default;

    template <typename Tag, typename std::enable_if<(std::is_base_of<link_tag, Tag>::value || std::is_base_of<stats_tag, Tag>::value)>::type * = nullptr>
    void dispatch_link(Tag, struct nlmsghdr * nlmsg)
    {
        if (nlmsg->nlmsg_type != RTM_NEWLINK)
            return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        auto * ifi = reinterpret_cast<struct ::ifinfomsg *>(NLMSG_DATA(nlmsg));
        std::size_t len = RTM_PAYLOAD(nlmsg);
        switch(ifi->ifi_family) {
            case AF_UNSPEC:
                dispatch_link_unspec(Tag(), ifi, len);
                break;
        }

#pragma GCC diagnostic pop
    }

    template <typename Tag, typename std::enable_if<!(std::is_base_of<link_tag, Tag>::value || std::is_base_of<stats_tag, Tag>::value)>::type * = nullptr>
    void dispatch_link(Tag, struct nlmsghdr *) {}

    template <typename Tag>
    void dispatch_link_unspec(Tag, struct ifinfomsg * ifi, std::size_t len)
    {
        int type = ifi->ifi_type;
        uint flags = ifi->ifi_flags;
        std::string ifname;
        acqua::network::linklayer_address addr;
        struct ::rtnl_link_stats * stats = nullptr;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case IFLA_IFNAME:
                    ifname = reinterpret_cast<char const *>(RTA_DATA(rta));
                    break;
                case IFLA_ADDRESS:
                    addr = addr.from_voidptr(RTA_DATA(rta));
                    break;
                case IFLA_MTU:
                    break;
                case IFLA_STATS:
                    stats = reinterpret_cast<struct ::rtnl_link_stats *>(RTA_DATA(rta));
                    break;
            }
        }

#pragma GCC diagnostic pop
        dispatch_link_changed(Tag(), ifname, addr, type, flags);
        dispatch_link_stats(Tag(), ifname, stats, type, flags);
    }

    template <typename Tag, typename std::enable_if<std::is_base_of<link_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_changed(Tag, std::string const & ifname, acqua::network::linklayer_address const & addr, int type, uint flags)
    {
        static_cast<Derived *>(this)->on_link(ifname, addr, type, flags);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<link_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_changed(Tag, std::string, acqua::network::linklayer_address, int, uint) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<stats_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_stats(Tag, std::string const & ifname, struct ::rtnl_link_stats * stats, int type, uint flags)
    {
        if (!stats)
            return;
        uint tx_packets = stats->tx_packets;
        uint tx_bytes = stats->tx_bytes;
        uint rx_packets = stats->rx_packets;
        uint rx_bytes = stats->rx_bytes;
        static_cast<Derived *>(this)->on_stats(ifname, tx_packets, tx_bytes, rx_packets, rx_bytes, type, flags);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<stats_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_stats(Tag, std::string, struct ::rtnl_link_stats *, int, uint) {}
};

} } }
