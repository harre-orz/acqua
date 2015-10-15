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

#include <iterator>
#include <memory>
#include <boost/system/error_code.hpp>
#include <acqua/network/linklayer_address.hpp>
#include <acqua/network/internet4_address.hpp>
#include <acqua/network/internet6_address.hpp>

namespace acqua { namespace network {

class interface
    : private boost::noncopyable
{
    using internal_value_type = ::ifaddrs;

    explicit interface(internal_value_type * ifa);

public:
    class iterator;

    void dump(std::ostream & os) const;

    //! インタフェース名を取得
    std::string name() const;

    bool is_up() const;

    bool is_loopback() const;

    bool is_running() const;

    bool has_broadcast() const;

    bool has_point_to_point() const;

    bool has_multicast() const;

    bool is_packet() const;

    bool is_v4() const;

    bool is_v6() const;

    internet4_address to_address_v4() const;

    internet4_address to_netmask_v4() const;

    internet4_address to_broadcast_v4() const;

    internet4_address to_point_to_point_v4() const;

    internet6_address to_address_v6() const;

    internet6_address to_netmask_v6() const;

    internet6_address to_broadcast_v6() const;

    internet6_address to_point_to_point_v6() const;

    //! IPv6 の場合のみ スコープID を返し、それ以外は -1 を返す.
    int scope_id() const;

    //! インタフェースの index 番号を取得する.
    //! VIPの場合、eth0:0 , eth0:1 といったIF名が１つの index に結びつくので注意すること
    int index() const;

    //! 物理アドレスを返す.
    linklayer_address physical_address(boost::system::error_code & ec) const;

    //! 物理アドレスを返す.
    linklayer_address physical_address() const;

    //! 物理アドレスを変更する.
    //! この処理を成功させるには特権権限が必要
    void physical_address(linklayer_address const & lladdr) const;

    //! 物理アドレスを変更する.
    //! この処理を成功させるには特権権限が必要
    void physical_address(linklayer_address const & lladdr, boost::system::error_code & ec) const;

    //! MTU値を取得する.
    int mtu() const;

    //! MTU値を取得する.
    int mtu(boost::system::error_code & ec) const;

    //! MTU値を変更する.
    //! この処理を成功させるには特権権限が必要
    //! MTU値を小さくし過ぎると、カーネルがクラッシュする恐れがある
    void mtu(int num) const;

    //! MTU値を変更する.
    //! この処理を成功させるにはは特権権限が必要
    //! MTU値を小さくし過ぎると、カーネルがクラッシュする恐れがある
    void mtu(int num, boost::system::error_code & ec) const;

    static iterator begin();

    static iterator end();

    friend bool operator==(interface const & lhs, interface const & rhs);

    friend bool operator!=(interface const & lhs, interface const & rhs);

private:
    internal_value_type * ifa_;
};


class interface::iterator
    : public std::iterator<std::forward_iterator_tag, interface>
{
    friend interface;

    iterator(internal_value_type * ifa);

public:
    iterator();

    iterator(iterator const & rhs);

    iterator(iterator && rhs);

    iterator & operator=(iterator const & rhs);

    iterator & operator=(iterator && rhs);

    interface const & operator*() const;

    interface const * operator->() const;

    iterator & operator++();

    iterator operator++(int);

    friend bool operator==(iterator const & lhs, iterator const & rhs);

    friend bool operator!=(iterator const & lhs, iterator const & rhs);

private:
    std::shared_ptr<internal_value_type> base_;
    interface value_;
};

} }

#include <acqua/network/impl/interface.ipp>
