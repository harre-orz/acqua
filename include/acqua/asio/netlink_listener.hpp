/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
}

#include <type_traits>
#include <functional>
#include <boost/asio/generic/raw_protocol.hpp>
#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/internet6_address.hpp>

namespace acqua { namespace asio {

/*!
  netlink_listener クラスで、インタフェースのUp/Down の取得するタグ.
 */
struct netlink_link_tag
{
    static const int group = RTMGRP_LINK;
};

/*!
  netlink_listener クラスで、IPアドレスの変化を取得するタグ.
 */
struct netlink_ipv4_ifaddr_tag
{
    static const int group = RTMGRP_IPV4_IFADDR;
};

/*!
  netlink_listener クラスで、ARPテーブルの更新・取得するタグ.
 */
struct netlink_neighbor_v4_tag
{
    static const int group = RTMGRP_NEIGH;
};

/*!
  netlink_listener クラスで、IPv6ネイバーテーブルを更新・取得するタグ.
 */
struct netlink_neighbor_v6_tag
{
    static const int group = RTMGRP_NEIGH;
};

/*!
  netlink_neighbor_v4_tag | netlink_neighbor_v6_tag
 */
struct netlink_neighbor_tag : netlink_neighbor_v4_tag, netlink_neighbor_v6_tag {};

template <typename Derived>
class netlink_listener
{
private:
    using protocol_type = boost::asio::generic::raw_protocol;
    using socket_type = typename protocol_type::socket;
    using endpoint_type = typename protocol_type::endpoint;

protected:
    using base_type = netlink_listener;

    ~netlink_listener() = default;

public:
    explicit netlink_listener(boost::asio::io_service & io_service)
        : socket_(io_service) {}

    void open(boost::system::error_code & ec)
    {
        socket_.open(protocol_type(AF_NETLINK, NETLINK_ROUTE), ec);
    }

    void bind(boost::system::error_code & ec)
    {
        struct sockaddr_nl nl;
        std::memset(&nl, 0, sizeof(nl));
        nl.nl_family = AF_NETLINK;
        nl.nl_groups =
              find_group<netlink_link_tag>(typename Derived::category())
            | find_group<netlink_ipv4_ifaddr_tag>(typename Derived::category())
            | find_group<netlink_neighbor_v4_tag>(typename Derived::category())
            | find_group<netlink_neighbor_v6_tag>(typename Derived::category());
        socket_.bind(endpoint_type(&nl, sizeof(nl)), ec);
    }

    void start(boost::system::error_code & ec)
    {
        if (!socket_.is_open()) {
            open(ec);
            if (ec) return;
            bind(ec);
            if (ec) return;
        }

        async_receive();
    }

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec);
    }

    void close(boost::system::error_code & ec)
    {
        socket_.close(ec);
    }

    void close()
    {
        boost::system::error_code ec;
        socket_.close();
        boost::asio::detail::throw_error(ec);
    }

    void get_neighbor_v4(boost::system::error_code & ec)
    {
        struct {
            struct ::nlmsghdr nlmsg;
            struct ::ndmsg ndm;
        } sendbuf;
        std::memset(&sendbuf, 0, sizeof(sendbuf));
        sendbuf.nlmsg.nlmsg_len = sizeof(sendbuf);
        sendbuf.nlmsg.nlmsg_pid = 0;
        sendbuf.nlmsg.nlmsg_seq = 0;
        sendbuf.nlmsg.nlmsg_flags = NLM_F_REQUEST;
        sendbuf.nlmsg.nlmsg_type = RTM_GETNEIGH;
        sendbuf.ndm.ndm_family = AF_INET;

        struct sockaddr_nl nl;
        std::memset(&nl, 0, sizeof(nl));
        nl.nl_family = AF_NETLINK;
        socket_.send_to(boost::asio::buffer(&sendbuf, sizeof(sendbuf)), endpoint_type(&nl, sizeof(nl)), 0, ec);
    }

private:
    void async_receive()
    {
        socket_.async_receive(boost::asio::buffer(buffer_), std::bind(&netlink_listener::on_receive, this, std::placeholders::_1, std::placeholders::_2));
    }

    void on_receive(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            return;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * nlmsg = reinterpret_cast<struct ::nlmsghdr *>(buffer_.data());
            NLMSG_OK(nlmsg, size); nlmsg = NLMSG_NEXT(nlmsg, size)) {
            if (nlmsg->nlmsg_type == NLMSG_DONE || nlmsg->nlmsg_type == NLMSG_ERROR) {
                break;
            }

            dispatch_link(typename Derived::category(), nlmsg);
            dispatch_neighbor(typename Derived::category(), nlmsg);
        }

#pragma GCC diagnostic pop
        async_receive();
    }

    template <typename Tag, typename T>
    static uint find_group(T) { return std::is_base_of<Tag, T>::value ? Tag::group : 0; }

    template <typename Tag, typename std::enable_if<std::is_base_of<netlink_link_tag, Tag>::value>::type * = nullptr>
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

    template <typename Tag, typename std::enable_if<!std::is_base_of<netlink_link_tag, Tag>::value>::type * = nullptr>
    void dispatch_link(Tag, struct nlmsghdr *) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<netlink_link_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_unspec(Tag, struct ifinfomsg * ifi, std::size_t len)
    {
        std::string ifname;
        acqua::network::linklayer_address ll_addr;
        int type = ifi->ifi_type;
        uint flags = ifi->ifi_flags;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = IFLA_RTA(ifi); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case IFLA_IFNAME:
                    ifname = reinterpret_cast<char const *>(RTA_DATA(rta));
                    break;
                case IFLA_ADDRESS:
                    ll_addr = ll_addr.from_voidptr(RTA_DATA(rta));
                    break;
                case IFLA_MTU:
                    break;
            }
        }

#pragma GCC diagnostic pop
        static_cast<Derived *>(this)->on_link(ifname, ll_addr, type, flags);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<netlink_link_tag, Tag>::value>::type * = nullptr>
    void dispatch_link_unspec(Tag, struct ifinfomsg *, std::size_t) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<netlink_neighbor_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor(Tag, struct nlmsghdr * nlmsg)
    {
        if (nlmsg->nlmsg_type != RTM_NEWNEIGH)
            return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        auto ndm = reinterpret_cast<struct ndmsg *>(NLMSG_DATA(nlmsg));
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

    template <typename Tag, typename std::enable_if<!std::is_base_of<netlink_neighbor_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor(Tag, struct nlmsghdr *) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<netlink_neighbor_v4_tag, Tag>::value>::type * = nullptr>
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

    template <typename Tag, typename std::enable_if<!std::is_base_of<netlink_neighbor_v4_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v4(Tag, struct ndmsg *, std::size_t) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<netlink_neighbor_v6_tag, Tag>::value>::type * = nullptr>
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

    template <typename Tag, typename std::enable_if<!std::is_base_of<netlink_neighbor_v6_tag, Tag>::value>::type * = nullptr>
    void dispatch_neighbor_v6(Tag, struct ndmsg *, std::size_t) {}

private:
    socket_type socket_;
    std::array<char, 4096> buffer_;
};

} }
