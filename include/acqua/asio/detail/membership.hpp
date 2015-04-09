#pragma once

extern "C" {
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
}

#include <acqua/asio/raw.hpp>
#include <acqua/network/linklayer_address.hpp>

namespace acqua { namespace asio { namespace detail {

template <int Level, int Name, int Type>
class membership
{
public:
    explicit membership(raw::endpoint const & ep)
    {
        std::memset(&mr_, 0, sizeof(mr_));
        mr_.mr_type = Type;
        mr_.mr_ifindex = ep.scope_id();
    }

    explicit membership(raw::endpoint const & ep, acqua::network::linklayer_address const & addr)
    {
        std::memset(&mr_, 0, sizeof(mr_));
        mr_.mr_type = Type;
        mr_.mr_ifindex = ep.scope_id();
        mr_.mr_alen = 6;
        std::memcpy(mr_.mr_address, reinterpret_cast<char const *>(&addr), 6);
    }

    int level(raw const &) const noexcept
    {
        return Level;
    }

    int name(raw const &) const noexcept
    {
        return Name;
    }

    void const * data(raw const &) const noexcept
    {
        return &mr_;
    }

    std::size_t size(raw const &) const noexcept
    {
        return sizeof(mr_);
    }

    void resize(raw const &) const noexcept
    {
    }

private:
    struct ::packet_mreq mr_;
};

} } }
