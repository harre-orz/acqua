/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <ifaddrs.h>
}

#include <memory>
#include <boost/system/error_code.hpp>
#include <acqua/exception/throw_error.hpp>
#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/internet6_address.hpp>
#include <acqua/network/detail/prefix_address.hpp>

namespace acqua { namespace network {

class interface
    : private boost::noncopyable
{
    using internal_value_type = ::ifaddrs;

    explicit interface(internal_value_type * ifa)
        : ifa_(ifa) {}

public:
    class iterator;
    friend iterator;
    static iterator begin();
    static iterator end();

    std::string name() const
    {
        return ifa_->ifa_name;
    }

    bool is_up() const
    {
        return ifa_->ifa_flags & IFF_UP;
    }

    bool is_loopback() const
    {
        return ifa_->ifa_flags & IFF_LOOPBACK;
    }

    bool is_running() const
    {
        return ifa_->ifa_flags & IFF_RUNNING;
    }

    bool has_broadcast() const
    {
        // IPv6 の場合、ifu_broadaddr が NULL のときがあるので、ここでもNULLチェックしておく
        return ifa_->ifa_flags & IFF_BROADCAST && ifa_->ifa_broadaddr;
    }

    bool has_point_to_point() const
    {
        return ifa_->ifa_flags & IFF_POINTOPOINT;
    }

    bool has_multicast() const
    {
        return ifa_->ifa_flags & IFF_MULTICAST;
    }

    bool is_stats() const
    {
        return ifa_->ifa_addr->sa_family == AF_PACKET;
    }

    bool is_v4() const
    {
        return ifa_->ifa_addr->sa_family == AF_INET;
    }

    internet4_address to_address_v4() const
    {
        return ifa_->ifa_addr
            ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_addr)->sin_addr)
            : internet4_address();
    }

    internet4_address to_netmask_v4() const
    {
        return ifa_->ifa_netmask
            ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_netmask)->sin_addr)
            : internet4_address();
    }

    internet4_address to_broadcast_v4() const
    {
        return ifa_->ifa_broadaddr
            ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_broadaddr)->sin_addr)
            : internet4_address();
    }

    internet4_address to_point_to_point_v4() const
    {
        return ifa_->ifa_dstaddr
            ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_dstaddr)->sin_addr)
            : internet4_address();
    }

    bool is_v6() const
    {
        return ifa_->ifa_addr->sa_family == AF_INET6;
    }

    internet6_address to_address_v6() const
    {
        return ifa_->ifa_addr
            ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_addr)
            : internet6_address();
    }

    internet6_address to_netmask_v6() const
    {
        return ifa_->ifa_addr
            ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_netmask)->sin6_addr)
            : internet6_address();
    }

    internet6_address to_broadcast_v6() const
    {
        return ifa_->ifa_broadaddr
            ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_broadaddr)->sin6_addr)
            : internet6_address();
    }

    internet6_address to_point_to_point_v6() const
    {
        return ifa_->ifa_dstaddr
            ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_dstaddr)->sin6_addr)
            : internet6_address();
    }

    /*!
      IPv6 の場合のみ スコープID を返し、それ以外は -1 を返す.
     */
    int scope_id() const
    {
        return is_v6()
            ? reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_scope_id
            : -1;
    }

    /*!
      物理アドレスを返す.
     */
    linklayer_address physical_address(boost::system::error_code & ec) const
    {
        struct ::ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, ifa_->ifa_name, sizeof(ifr.ifr_name)-1);

        int fd;
        if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            ec.assign(errno, boost::system::generic_category());
            return linklayer_address();
        }

        if (::ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
            ec.assign(errno, boost::system::generic_category());
            ::close(fd);
            return linklayer_address();
        }
        ::close(fd);
        return linklayer_address(ifr.ifr_hwaddr.sa_data);
    }

    /*!
      物理アドレスを返す.
     */
    linklayer_address physical_address() const
    {
        boost::system::error_code ec;
        auto addr = physical_address(ec);
        acqua::exception::throw_error(ec, "physical_address");
        return addr;
    }

    friend bool operator==(interface const & lhs, interface const & rhs)
    {
        return lhs.ifa_ == rhs.ifa_;
    }

    friend bool operator!=(interface const & lhs, interface const & rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream & operator<<(std::ostream & os, interface const & rhs)
    {
        os << rhs.name();
        if (rhs.is_v4()) {
            os << " inet " << rhs.to_address_v4() << '/' << (int)netmask_length(rhs.to_netmask_v4());
            if (rhs.has_broadcast()) os << " via " << rhs.to_broadcast_v4();
            if (rhs.has_point_to_point()) os << " p2p " << rhs.to_point_to_point_v4();
        }
        else if (rhs.is_v6()) {
            os << " inet6 " << rhs.to_address_v6() << '/' << (int)netmask_length(rhs.to_netmask_v6()) << " scopeid " << rhs.scope_id();
            if (rhs.has_broadcast()) os << " via " << rhs.to_broadcast_v6();
            if (rhs.has_point_to_point()) os << " p2p " << rhs.to_point_to_point_v6();
        }
        return os;
    }

private:
    internal_value_type * ifa_;
};


class interface::iterator
{
    friend interface;

    iterator(internal_value_type * ifa)
        : base_(ifa, [](internal_value_type * ifa) { ::freeifaddrs(ifa); })
        , value_(ifa) {}

public:
    iterator()
        : base_(nullptr)
        , value_(nullptr) {}

    iterator(iterator const & rhs)
        : base_(rhs.base_)
        , value_(rhs.value_.ifa_) {}

    interface const & operator*() const
    {
        return value_;
    }

    interface const * operator->() const
    {
        return &value_;
    }

    friend bool operator==(iterator const & lhs, iterator const & rhs)
    {
        return lhs.value_ == rhs.value_;
    }

    friend bool operator!=(iterator const & lhs, iterator const & rhs)
    {
        return !(lhs == rhs);
    }

    iterator & operator++()
    {
        value_.ifa_ = value_.ifa_->ifa_next;
        return *this;
    }

    iterator operator++(int)
    {
        iterator it(*this);
        operator++();
        return it;
    }

private:
    std::shared_ptr<internal_value_type> base_;
    interface value_;
};


inline interface::iterator interface::begin()
{
    internal_value_type * ifa;
    ::getifaddrs(&ifa);
    return iterator(ifa);
}


inline interface::iterator interface::end()
{
    return iterator();
}

} }
