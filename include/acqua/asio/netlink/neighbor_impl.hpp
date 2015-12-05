#pragma once

#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/internet6_address.hpp>
#include <acqua/asio/netlink/categories.hpp>

namespace acqua { namespace asio { namespace netlink {

template <typename Derived>
class neighbor_impl
{
protected:
    ~neighbor_impl() = default;

    template <typename Tag, typename std::enable_if<(std::is_base_of<neighbor_v4_tag, Tag>::value || std::is_base_of<neighbor_v6_tag, Tag>::value)>::type * = nullptr>
    void dispatch_neighbor(Tag, struct nlmsghdr * nlmsg)
    {
        if (nlmsg->nlmsg_type != RTM_NEWNEIGH)
            return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        auto ndm = reinterpret_cast<struct ::ndmsg *>(NLMSG_DATA(nlmsg));
        std::size_t len = RTM_PAYLOAD(nlmsg);
        switch(ndm->ndm_family) {
            case AF_INET:
                dispatch_neighbor_v4(Tag(), ndm, len);
                break;
            case AF_INET6:
                dispatch_neighbor_v6(Tag(), ndm, len);
                break;
        }

#pragma GCC diagnostic pop
    }

    template <typename Tag, typename std::enable_if<!(std::is_base_of<neighbor_v4_tag, Tag>::value || std::is_base_of<neighbor_v6_tag, Tag>::value)>::type * = nullptr>
    void dispatch_neighbor(Tag, struct nlmsghdr *) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<neighbor_v4_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v4(Tag, struct ndmsg * ndm, std::size_t len)
    {
        acqua::network::linklayer_address ll_addr;
        acqua::network::internet4_address in_addr;
        uint state = ndm->ndm_state;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(ndm)) + NLMSG_ALIGN(sizeof(*ndm)));
            RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case NDA_DST:
                    in_addr = in_addr.from_voidptr(RTA_DATA(rta));
                    break;
                case NDA_LLADDR:
                    ll_addr = ll_addr.from_voidptr(RTA_DATA(rta));
                    break;
            }
        }

#pragma GCC diagnostic pop
        static_cast<Derived *>(this)->on_neighbor(in_addr, ll_addr, state);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<neighbor_v4_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v4(Tag, struct ndmsg *, std::size_t) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<neighbor_v6_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v6(Tag, struct ndmsg * ndm, std::size_t len)
    {
        acqua::network::linklayer_address ll_addr;
        acqua::network::internet6_address in_addr;
        uint state = ndm->ndm_state;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = reinterpret_cast<struct rtattr *>((reinterpret_cast<char *>(ndm)) + NLMSG_ALIGN(sizeof(*ndm)));
            RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case NDA_DST:
                    in_addr = in_addr.from_voidptr(RTA_DATA(rta));
                    break;
                case NDA_LLADDR:
                    ll_addr = ll_addr.from_voidptr(RTA_DATA(rta));
                    break;
            }
        }

#pragma GCC diagnostic pop
        static_cast<Derived *>(this)->on_neighbor(in_addr, ll_addr, state);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<neighbor_v6_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v6(Tag, struct ndmsg *, std::size_t) {}

// public:
//     void get_neighbor_v4(boost::system::error_code & ec)
//     {
//         struct {
//             struct ::nlmsghdr nlmsg;
//             struct ::ndmsg ndm;
//         } sendbuf;
//         std::memset(&sendbuf, 0, sizeof(sendbuf));
//         sendbuf.nlmsg.nlmsg_len = sizeof(sendbuf);
//         sendbuf.nlmsg.nlmsg_pid = 0;
//         sendbuf.nlmsg.nlmsg_seq = 0;
//         sendbuf.nlmsg.nlmsg_flags = NLM_F_REQUEST;
//         sendbuf.nlmsg.nlmsg_type = RTM_GETNEIGH;
//         sendbuf.ndm.ndm_family = AF_INET;

//         struct sockaddr_nl nl;
//         std::memset(&nl, 0, sizeof(nl));
//         nl.nl_family = AF_NETLINK;
//         socket_.send_to(boost::asio::buffer(&sendbuf, sizeof(sendbuf)), endpoint_type(&nl, sizeof(nl)), 0, ec);
//     }
};

} } }
