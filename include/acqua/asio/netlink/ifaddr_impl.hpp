#pragma once

#include <acqua/asio/netlink/categories.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/internet6_address.hpp>

namespace acqua { namespace asio { namespace netlink {

template <typename Derived>
class ifaddr_impl
{
protected:
    ~ifaddr_impl() = default;

    template <typename Tag, typename std::enable_if<(std::is_base_of<ifaddr_v4_tag, Tag>::value || std::is_base_of<ifaddr_v6_tag, Tag>::value)>::type * = nullptr>
    void dispatch_ifaddr(Tag, struct nlmsghdr * nlmsg)
    {
        if (nlmsg->nlmsg_type != RTM_NEWADDR)
            return;

        auto ifa = reinterpret_cast<struct ::ifaddrmsg *>(NLMSG_DATA(nlmsg));
        std::size_t len = RTM_PAYLOAD(nlmsg);
        switch(ifa->ifa_family) {
            case AF_INET:
                dispatch_ifaddr_v4(Tag(), ifa, len);
                break;
            case AF_INET6:
                dispatch_ifaddr_v6(Tag(), ifa, len);
                break;
        }
    }

    template <typename Tag, typename std::enable_if<!(std::is_base_of<ifaddr_v4_tag, Tag>::value || std::is_base_of<ifaddr_v6_tag, Tag>::value)>::type * = nullptr>
    void dispatch_ifaddr(Tag, struct nlmsghdr *) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<ifaddr_v4_tag, Tag>::value>::type * = nullptr>
    void dispatch_ifaddr_v4(Tag, struct ifaddrmsg * ifa, std::size_t len)
    {
        acqua::network::internet4_address addr;
        std::string label;
        uint prefixlen = ifa->ifa_prefixlen;
        uint flags = ifa->ifa_flags;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = IFA_RTA(ifa); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case IFA_ADDRESS:
                    addr = addr.from_voidptr(RTA_DATA(rta));
                    break;
                case IFA_LABEL:
                    label = static_cast<char const *>(RTA_DATA(rta));
                    break;
            }
        }

#pragma GCC diagnostic pop
        static_cast<Derived *>(this)->on_ifaddr(addr, label, prefixlen, flags);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<ifaddr_v4_tag, Tag>::value>::type * = nullptr>
    void dispatch_ifaddr_v4(Tag, struct ifaddrmsg *, std::size_t) {}

    template <typename Tag, typename std::enable_if<std::is_base_of<ifaddr_v6_tag, Tag>::value>::type * = nullptr>
    void dispatch_ifaddr_v6(Tag, struct ifaddrmsg * ifa, std::size_t len)
    {
        acqua::network::internet6_address addr;
        std::string label;
        uint prefixlen = ifa->ifa_prefixlen;
        uint flags = ifa->ifa_flags;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * rta = IFA_RTA(ifa); RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
            switch(rta->rta_type) {
                case IFA_ADDRESS:
                    addr = addr.from_voidptr(RTA_DATA(rta));
                    break;
                case IFA_LABEL:
                    label = static_cast<char const *>(RTA_DATA(rta));
                    break;
            }
        }

#pragma GCC diagnostic pop
        static_cast<Derived *>(this)->on_ifaddr(addr, label, prefixlen, flags);
    }

    template <typename Tag, typename std::enable_if<!std::is_base_of<ifaddr_v6_tag, Tag>::value>::type * = nullptr>
    void dispatch_ifaddr_v6(Tag, struct ifaddrmsg *, std::size_t) {}
};

} } }
