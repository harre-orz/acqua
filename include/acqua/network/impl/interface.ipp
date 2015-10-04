/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/interface.hpp>
#include <acqua/network/detail/ifreq_opts.hpp>

namespace acqua { namespace network {

interface::interface(internal_value_type * ifa)
    : ifa_(ifa)
{
}

void interface::dump(std::ostream & os) const
{
    os << name();
    if (is_v4()) {
        os << " inet " << to_address_v4() << "/" << netmask_length(to_netmask_v4());
        if (has_broadcast()) os << " via " << to_broadcast_v4();
        if (has_point_to_point()) os << " to " << to_point_to_point_v4();
    }
    if (is_v6()) {
        os << " inet6 " << to_address_v6() << "/" << netmask_length(to_netmask_v6());
        if (has_broadcast()) os << " via " << to_broadcast_v6();
        if (has_point_to_point()) os << " to " << to_point_to_point_v6();
        os << " scope-id " << scope_id();
    }
    os << " < ";
    if (is_up()) os << "UP ";
    if (is_loopback()) os << "LOOPBACK ";
    if (is_running()) os << "RUNNING ";
    if (has_multicast()) os << "MULTICAST ";
    os << ">";
}

std::string interface::name() const
{
    return ifa_->ifa_name;
}

bool interface::is_up() const
{
    return ifa_->ifa_flags & IFF_UP;
}

bool interface::is_loopback() const
{
    return ifa_->ifa_flags & IFF_LOOPBACK;
}

bool interface::is_running() const
{
    return ifa_->ifa_flags & IFF_RUNNING;
}

bool interface::has_broadcast() const
{
    // IPv6 の場合、ifu_broadaddr が NULL のときがあるので、ここでもNULLチェックしておく
    return ifa_->ifa_flags & IFF_BROADCAST && ifa_->ifa_broadaddr;
}

bool interface::has_point_to_point() const
{
    // IPv6 の場合、ifu_dstaddr が NULL のときがあるので、ここでもNULLチェックしておく
    return ifa_->ifa_flags & IFF_POINTOPOINT && ifa_->ifa_dstaddr;
}

bool interface::has_multicast() const
{
    return ifa_->ifa_flags & IFF_MULTICAST;
}

bool interface::is_packet() const
{
    return ifa_->ifa_addr->sa_family == AF_PACKET;
}

bool interface::is_v4() const
{
    return ifa_->ifa_addr->sa_family == AF_INET;
}

bool interface::is_v6() const
{
    return ifa_->ifa_addr->sa_family == AF_INET6;
}

internet4_address interface::to_address_v4() const
{
    return is_v4()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_addr)->sin_addr)
        : internet4_address();
}

internet4_address interface::to_netmask_v4() const
{
    return is_v4()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_netmask)->sin_addr)
        : internet4_address();
}

internet4_address interface::to_broadcast_v4() const
{
    return is_v4() && has_broadcast()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_broadaddr)->sin_addr)
        : internet4_address();
}

internet4_address interface::to_point_to_point_v4() const
{
    return (is_v4() && has_point_to_point())
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_dstaddr)->sin_addr)
        : internet4_address();
}

internet6_address interface::to_address_v6() const
{
    return is_v6()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_addr)
        : internet6_address();
}

internet6_address interface::to_netmask_v6() const
{
    return is_v6()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_netmask)->sin6_addr)
        : internet6_address();
}

internet6_address interface::to_broadcast_v6() const
{
    return is_v6() && has_broadcast()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_broadaddr)->sin6_addr)
        : internet6_address();
}

internet6_address interface::to_point_to_point_v6() const
{
    return is_v6() && has_point_to_point()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_dstaddr)->sin6_addr)
        : internet6_address();
}

int interface::scope_id() const
{
    return is_v6()
        ? reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_scope_id
        : -1;
}

int interface::index() const
{
    return ::if_nametoindex(ifa_->ifa_name);
}

linklayer_address interface::physical_address() const
{
    boost::system::error_code ec;
    auto addr = physical_address(ec);
    boost::asio::detail::throw_error(ec, "physical_address");
    return addr;
}

linklayer_address interface::physical_address(boost::system::error_code & ec) const
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    return opts.get_lladdr(ec);
}

void interface::physical_address(linklayer_address const & lladdr) const
{
    boost::system::error_code ec;
    physical_address(lladdr, ec);
    boost::asio::detail::throw_error(ec, "physical_address");
}

void interface::physical_address(linklayer_address const & lladdr, boost::system::error_code & ec) const
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    opts.set_lladdr(lladdr, ec);
}

int interface::mtu() const
{
    boost::system::error_code ec;
    auto val = mtu(ec);
    boost::asio::detail::throw_error(ec, "mtu");
    return val;
}

int interface::mtu(boost::system::error_code & ec) const
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    return opts.get_mtu(ec);
}

void interface::mtu(int num) const
{
    boost::system::error_code ec;
    mtu(num, ec);
    boost::asio::detail::throw_error(ec, "mtu");
}

void interface::mtu(int num, boost::system::error_code & ec) const
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    opts.set_mtu(num, ec);
}

interface::iterator interface::begin()
{
    internal_value_type * ifa;
    ::getifaddrs(&ifa);
    return iterator(ifa);
}

interface::iterator interface::end()
{
    return iterator();
}

bool operator==(interface const & lhs, interface const & rhs)
{
    return lhs.ifa_ == rhs.ifa_;
}

bool operator!=(interface const & lhs, interface const & rhs)
{
    return lhs.ifa_ != rhs.ifa_;
}

interface::iterator::iterator(internal_value_type * ifa)
    : base_(ifa, [](internal_value_type * ifa) { ::freeifaddrs(ifa); })
    , value_(ifa)
{
}

interface::iterator::iterator()
    : base_(nullptr)
    , value_(nullptr)
{
}

interface::iterator::iterator(iterator const & rhs)
    : base_(rhs.base_)
    , value_(rhs.value_.ifa_)
{
}

interface::iterator::iterator(iterator && rhs)
    : base_(std::move(rhs.base_))
    , value_(rhs.value_.ifa_)
{
}

interface::iterator & interface::iterator::operator=(iterator const & rhs)
{
    base_ = rhs.base_;
    value_.ifa_ = rhs.value_.ifa_;
    return *this;
}

interface::iterator & interface::iterator::operator=(iterator && rhs)
{
    base_ = std::move(rhs.base_);
    value_.ifa_ = rhs.value_.ifa_;
    return *this;
}

interface const & interface::iterator::operator*() const
{
    return value_;
}

interface const * interface::iterator::operator->() const
{
    return &value_;
}

interface::iterator & interface::iterator::operator++()
{
    value_.ifa_ = value_.ifa_->ifa_next;
    return *this;
}

interface::iterator interface::iterator::operator++(int)
{
    iterator it(*this);
    operator++();
    return it;
}

bool operator==(interface::iterator const & lhs, interface::iterator const & rhs)
{
    return lhs.value_ == rhs.value_;
}

bool operator!=(interface::iterator const & lhs, interface::iterator const & rhs)
{
    return lhs.value_ != rhs.value_;
}

} }
