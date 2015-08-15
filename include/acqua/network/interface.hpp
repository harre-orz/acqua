#pragma once

extern "C" {
#include <ifaddrs.h>
}

#include <memory>
#include <boost/system/error_code.hpp>
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

    bool is_packet() const
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

    int scope_id() const
    {
        return is_v6()
            ? reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_scope_id
            : -1;
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
public:
    iterator()
        : base_(nullptr)
        , value_(nullptr) {}

    iterator(interface const & ifa)
        : base_(ifa.ifa_, [](internal_value_type * ifa) { ::freeifaddrs(ifa); })
        , value_(ifa.ifa_) {}

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
        return lhs.equal_to(rhs);
    }

    friend bool operator!=(iterator const & lhs, iterator const & rhs)
    {
        return !lhs.equal_to(rhs);
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
    bool equal_to(iterator const & rhs) const
    {
        return value_.ifa_ == rhs.value_.ifa_;
    }

    std::shared_ptr<internal_value_type> base_;
    interface value_;
};


inline interface::iterator interface::begin()
{
    internal_value_type * ifa;
    ::getifaddrs(&ifa);
    return interface(ifa);
}


inline interface::iterator interface::end()
{
    return iterator();
}

} }
