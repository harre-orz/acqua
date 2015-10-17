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

inline constexpr interface::interface(internal_value_type * ifa) noexcept
    : ifa_(ifa)
{
}

inline void interface::dump(std::ostream & os) const
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

inline std::string interface::name() const
{
    return ifa_->ifa_name;
}

inline bool interface::is_up() const noexcept
{
    return ifa_->ifa_flags & IFF_UP;
}

inline bool interface::is_loopback() const noexcept
{
    return ifa_->ifa_flags & IFF_LOOPBACK;
}

inline bool interface::is_running() const noexcept
{
    return ifa_->ifa_flags & IFF_RUNNING;
}

inline bool interface::has_broadcast() const noexcept
{
    // IPv6 の場合、ifu_broadaddr が NULL のときがあるので、ここでもNULLチェックしておく
    return ifa_->ifa_flags & IFF_BROADCAST && ifa_->ifa_broadaddr;
}

inline bool interface::has_point_to_point() const noexcept
{
    // IPv6 の場合、ifu_dstaddr が NULL のときがあるので、ここでもNULLチェックしておく
    return ifa_->ifa_flags & IFF_POINTOPOINT && ifa_->ifa_dstaddr;
}

inline bool interface::has_multicast() const noexcept
{
    return ifa_->ifa_flags & IFF_MULTICAST;
}

inline bool interface::is_packet() const noexcept
{
    return ifa_->ifa_addr->sa_family == AF_PACKET;
}

inline bool interface::is_v4() const noexcept
{
    return ifa_->ifa_addr->sa_family == AF_INET;
}

inline bool interface::is_v6() const noexcept
{
    return ifa_->ifa_addr->sa_family == AF_INET6;
}

inline internet4_address interface::to_address_v4() const noexcept
{
    return is_v4()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_addr)->sin_addr)
        : internet4_address();
}

inline internet4_address interface::to_netmask_v4() const noexcept
{
    return is_v4()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_netmask)->sin_addr)
        : internet4_address();
}

inline internet4_address interface::to_broadcast_v4() const noexcept
{
    return is_v4() && has_broadcast()
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_broadaddr)->sin_addr)
        : internet4_address();
}

inline internet4_address interface::to_point_to_point_v4() const noexcept
{
    return (is_v4() && has_point_to_point())
        ? internet4_address(reinterpret_cast<struct ::sockaddr_in const *>(ifa_->ifa_dstaddr)->sin_addr)
        : internet4_address();
}

inline internet6_address interface::to_address_v6() const noexcept
{
    return is_v6()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_addr)
        : internet6_address();
}

inline internet6_address interface::to_netmask_v6() const noexcept
{
    return is_v6()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_netmask)->sin6_addr)
        : internet6_address();
}

inline internet6_address interface::to_broadcast_v6() const noexcept
{
    return is_v6() && has_broadcast()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_broadaddr)->sin6_addr)
        : internet6_address();
}

inline internet6_address interface::to_point_to_point_v6() const noexcept
{
    return is_v6() && has_point_to_point()
        ? internet6_address(reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_dstaddr)->sin6_addr)
        : internet6_address();
}

inline int interface::scope_id() const noexcept
{
    return is_v6()
        ? reinterpret_cast<struct ::sockaddr_in6 const *>(ifa_->ifa_addr)->sin6_scope_id
        : -1;
}

inline int interface::index() const noexcept
{
    return ::if_nametoindex(ifa_->ifa_name);
}

inline linklayer_address interface::physical_address() const
{
    boost::system::error_code ec;
    auto addr = physical_address(ec);
    boost::asio::detail::throw_error(ec, "physical_address");
    return addr;
}

inline linklayer_address interface::physical_address(boost::system::error_code & ec) const noexcept
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    return opts.get_lladdr(ec);
}

inline void interface::physical_address(linklayer_address const & lladdr) const
{
    boost::system::error_code ec;
    physical_address(lladdr, ec);
    boost::asio::detail::throw_error(ec, "physical_address");
}

inline void interface::physical_address(linklayer_address const & lladdr, boost::system::error_code & ec) const noexcept
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    opts.set_lladdr(lladdr, ec);
}

inline int interface::mtu() const
{
    boost::system::error_code ec;
    auto val = mtu(ec);
    boost::asio::detail::throw_error(ec, "mtu");
    return val;
}

inline int interface::mtu(boost::system::error_code & ec) const noexcept
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    return opts.get_mtu(ec);
}

inline void interface::mtu(int num) const
{
    boost::system::error_code ec;
    mtu(num, ec);
    boost::asio::detail::throw_error(ec, "mtu");
}

inline void interface::mtu(int num, boost::system::error_code & ec) const noexcept
{
    detail::ifreq_opts opts(ifa_->ifa_name);
    opts.set_mtu(num, ec);
}

inline auto interface::begin() noexcept -> iterator
{
    internal_value_type * ifa;
    ::getifaddrs(&ifa);
    return iterator(ifa);
}

inline auto interface::end() noexcept -> iterator
{
    return iterator();
}

inline bool operator==(interface const & lhs, interface const & rhs) noexcept
{
    return lhs.ifa_ == rhs.ifa_;
}

inline bool operator!=(interface const & lhs, interface const & rhs) noexcept
{
    return lhs.ifa_ != rhs.ifa_;
}

inline interface::iterator::iterator(internal_value_type * ifa) noexcept
    : base_(ifa, [](internal_value_type * ifa) { ::freeifaddrs(ifa); })
    , value_(ifa)
{
}

inline constexpr interface::iterator::iterator() noexcept
    : base_(nullptr)
    , value_(nullptr)
{
}

inline interface::iterator::iterator(iterator const & rhs) noexcept
    : base_(rhs.base_)
    , value_(rhs.value_.ifa_)
{
}

inline interface::iterator::iterator(iterator && rhs) noexcept
    : base_(std::move(rhs.base_))
    , value_(rhs.value_.ifa_)
{
}

inline interface::iterator & interface::iterator::operator=(iterator const & rhs) noexcept
{
    base_ = rhs.base_;
    value_.ifa_ = rhs.value_.ifa_;
    return *this;
}

inline interface::iterator & interface::iterator::operator=(iterator && rhs) noexcept
{
    base_ = std::move(rhs.base_);
    value_.ifa_ = rhs.value_.ifa_;
    return *this;
}

inline interface const & interface::iterator::operator*() const noexcept
{
    return value_;
}

inline interface const * interface::iterator::operator->() const noexcept
{
    return &value_;
}

inline interface::iterator & interface::iterator::operator++() noexcept
{
    value_.ifa_ = value_.ifa_->ifa_next;
    return *this;
}

inline interface::iterator interface::iterator::operator++(int) noexcept
{
    iterator it(*this);
    operator++();
    return it;
}

inline bool operator==(interface::iterator const & lhs, interface::iterator const & rhs) noexcept
{
    return lhs.value_ == rhs.value_;
}

inline bool operator!=(interface::iterator const & lhs, interface::iterator const & rhs) noexcept
{
    return lhs.value_ != rhs.value_;
}

} }
