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

namespace acqua { namespace asio {

class raw
{
public:
    typedef raw protocol_type;
    typedef boost::asio::basic_raw_socket<raw> socket;
    typedef boost::asio::ip::basic_endpoint<raw> endpoint;

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
    typedef struct ::sockaddr data_type;
    typedef acqua::asio::raw protocol_type;

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

    unsigned int scope_id() const noexcept
    {
        return sll_.sll_ifindex;
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, basic_endpoint const & rhs)
    {
        char buf[IF_NAMESIZE];
        if (::if_indextoname(rhs.scope_id(), buf) != 0)
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
