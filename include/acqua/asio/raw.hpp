/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <net/if.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <netpacket/packet.h>
}

#include <cstring>
#include <iostream>
#include <boost/asio/basic_raw_socket.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <acqua/exception/throw_error.hpp>
#include <acqua/network/linklayer_address.hpp>
#include <acqua/asio/detail/membership.hpp>

namespace acqua { namespace asio {

namespace socket_base {

using add_promisc = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_PROMISC>;
using del_promisc = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_PROMISC>;
using add_multicast = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_MULTICAST>;
using del_multicast = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_MULTICAST>;
using add_allmulti = detail::membership<SOL_PACKET, PACKET_ADD_MEMBERSHIP, PACKET_MR_ALLMULTI>;
using del_allmulti = detail::membership<SOL_PACKET, PACKET_DROP_MEMBERSHIP, PACKET_MR_ALLMULTI>;

}

class raw
{
public:
    using protocol_type = raw;
    using socket = boost::asio::basic_raw_socket<raw>;
    using endpoint = boost::asio::ip::basic_endpoint<raw>;
    using address = acqua::network::linklayer_address;

    int family() const noexcept
    {
        return AF_PACKET;
    }

    int type() const noexcept
    {
        return SOCK_RAW;
    }

    int protocol() const noexcept
    {
        return ntohs(ETH_P_ALL);
    }
};

} }


namespace boost { namespace asio { namespace ip {

template <>
class basic_endpoint<acqua::asio::raw>
{
public:
    using data_type = ::sockaddr;
    using protocol_type = acqua::asio::raw;
    using address_type = typename protocol_type::address;

    basic_endpoint() noexcept
    {
        std::memset(&sll_, 0, sizeof(sll_));
    }

    basic_endpoint(basic_endpoint const &) noexcept = default;

    basic_endpoint(basic_endpoint &&) noexcept = default;

    basic_endpoint(char const * ifname, boost::system::error_code & ec) noexcept
    {
        setup(ifname, ec);
    }

    basic_endpoint(char const * ifname)
    {
        boost::system::error_code ec;
        setup(ifname, ec);
        acqua::exception::throw_error(ec, "if_nametoindex");
    }

    basic_endpoint(std::string const & str, boost::system::error_code & ec) noexcept
    {
        setup(str.c_str(), ec);
    }

    basic_endpoint(std::string const & str)
    {
        boost::system::error_code ec;
        setup(str.c_str(), ec);
        acqua::exception::throw_error(ec, "if_nametoindex");
    }

    bool operator==(basic_endpoint const & rhs) const noexcept
    {
        return scope_id() == rhs.scope_id();
    }

    bool operator!=(basic_endpoint const & rhs) const noexcept
    {
        return scope_id() != rhs.scope_id();
    }

    protocol_type protocol() const noexcept
    {
        return protocol_type();
    }

    std::size_t size() const noexcept
    {
        return sizeof(*this);
    }

    std::size_t capacity() const noexcept
    {
        return sizeof(*this);
    }

    void resize(std::size_t) noexcept
    {
    }

    data_type * data() noexcept
    {
        return reinterpret_cast<data_type *>(&sll_);
    }

    data_type const * data() const noexcept
    {
        return reinterpret_cast<data_type const *>(&sll_);
    }

    address_type address(boost::system::error_code & ec) const noexcept
    {
        struct ::ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        if (::if_indextoname(sll_.sll_ifindex, ifr.ifr_name) == nullptr) {
            ec.assign(errno, boost::system::generic_category());
            return address_type();
        }

        int fd;
        if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            ec.assign(errno, boost::system::generic_category());
            return address_type();
        }

        if (::ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
            ec.assign(errno, boost::system::generic_category());
            ::close(fd);
            return address_type();
        }

        ::close(fd);
        return address_type(ifr.ifr_hwaddr.sa_data);
    }

    address_type address() noexcept
    {
        boost::system::error_code ec;
        auto addr = address(ec);
        acqua::exception::throw_error(ec, "address");
        return addr;
    }

    unsigned int scope_id() const noexcept
    {
        return sll_.sll_ifindex;
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_endpoint const & rhs)
    {
        char buf[IF_NAMESIZE];
        if (::if_indextoname(rhs.scope_id(), buf) == nullptr)
            std::strcpy(buf, "null");
        std::copy_n(buf, std::strlen(buf), std::ostreambuf_iterator<char>(os));
        return os;
    }

    friend std::size_t hash_value(basic_endpoint const & rhs) noexcept
    {
        return rhs.scope_id();
    }

private:
    void setup(char const * ifname, boost::system::error_code & ec) noexcept
    {
        std::memset(&sll_, 0, sizeof(sll_));
        sll_.sll_family = protocol().family();
        sll_.sll_protocol = protocol().protocol();
        if ((sll_.sll_ifindex = ::if_nametoindex(ifname)) == 0)
            ec.assign(errno, boost::system::generic_category());
    }

private:
    struct ::sockaddr_ll sll_;
};

} } }
