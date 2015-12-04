/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

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

namespace acqua { namespace asio { namespace socket_base {

template <int Level, int Name, int Type>
class membership
{
public:
    explicit membership(raw::endpoint const & ep)
    {
        std::memset(&mr_, 0, sizeof(mr_));
        mr_.mr_type = Type;
        mr_.mr_ifindex = ep.id();
    }

    explicit membership(raw::endpoint const & ep, raw::address const & addr)
    {
        std::memset(&mr_, 0, sizeof(mr_));
        mr_.mr_type = Type;
        mr_.mr_ifindex = ep.id();
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
