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
#include <acqua/network/linklayer_address.hpp>

namespace acqua { namespace asio {

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

    friend constexpr bool operator==(raw const &, raw const &)
    {
        return true;
    }

    friend constexpr bool operator!=(raw const & lhs, raw const & rhs)
    {
        return !(lhs == rhs);
    }
};

} }


namespace boost { namespace asio { namespace ip {

/*!
  RAWソケット用の endpoint クラス.

  仮想NICを指定したしても、物理NICの名称になるので注意。
  例えば eth0:1 という仮想NICがあっても、std::cout << ep での結果は eth0 になる。
 */
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
        boost::asio::detail::throw_error(ec, "if_nametoindex");
    }

    basic_endpoint(std::string const & str, boost::system::error_code & ec) noexcept
    {
        setup(str.c_str(), ec);
    }

    basic_endpoint(std::string const & str)
    {
        boost::system::error_code ec;
        setup(str.c_str(), ec);
        boost::asio::detail::throw_error(ec, "if_nametoindex");
    }

    friend bool operator==(basic_endpoint const & lhs, basic_endpoint const & rhs) noexcept
    {
        return lhs.protocol() == rhs.protocol() && lhs.id() == rhs.id();
    }

    friend bool operator!=(basic_endpoint const & lhs, basic_endpoint const & rhs) noexcept
    {
        return !(lhs == rhs);
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

    int id() const
    {
        return sll_.sll_ifindex;
    }

    address_type address(boost::system::error_code &) const noexcept
    {
        return address();
    }

    address_type address() const noexcept
    {
        return address_type::from_voidptr(sll_.sll_addr);
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_endpoint const & rhs)
    {
        char buf[IF_NAMESIZE];
        if (::if_indextoname(static_cast<uint>(rhs.id()), buf) == nullptr)
            std::strcpy(buf, "null");
        std::copy_n(buf, std::strlen(buf), std::ostreambuf_iterator<char>(os));
        return os;
    }

    friend std::size_t hash_value(basic_endpoint const & rhs) noexcept
    {
        return static_cast<std::size_t>(rhs.id());
    }

private:
    void setup(char const * ifname, boost::system::error_code & ec) noexcept
    {
        std::memset(&sll_, 0, sizeof(sll_));
        sll_.sll_family = static_cast<std::uint16_t>(protocol().family());
        sll_.sll_protocol = static_cast<std::uint16_t>(protocol().protocol());
        if ((sll_.sll_ifindex = static_cast<int>(::if_nametoindex(ifname))) == 0) {
            ec.assign(errno, boost::system::generic_category());
            return;
        }

        struct ::ifreq ifr;
        std::memset(&ifr, 0, sizeof(ifr));
        std::strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);

        int fd;
        if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            ec.assign(errno, boost::system::generic_category());
            return;
        }

        if (::ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
            ec.assign(errno, boost::system::generic_category());
        }

        ::close(fd);
        std::memcpy(sll_.sll_addr, &ifr.ifr_hwaddr, 6);
    }

private:
    struct ::sockaddr_ll sll_;
};

} } }

#include <acqua/asio/socket_base/raw_options.hpp>
