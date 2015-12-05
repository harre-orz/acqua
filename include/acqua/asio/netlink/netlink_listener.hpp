/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
}

#include <type_traits>
#include <functional>
#include <boost/system/error_code.hpp>
#include <boost/asio/generic/raw_protocol.hpp>
#include <acqua/asio/netlink/categories.hpp>
#include <acqua/asio/netlink/link_impl.hpp>
#include <acqua/asio/netlink/ifaddr_impl.hpp>
#include <acqua/asio/netlink/neighbor_impl.hpp>

namespace acqua { namespace asio { namespace netlink {

template <typename Derived>
class netlink_listener
    : private link_impl<Derived>, private ifaddr_impl<Derived>, private neighbor_impl<Derived>
{
    friend link_impl<Derived>;
    friend ifaddr_impl<Derived>;
    friend neighbor_impl<Derived>;

private:
    using protocol_type = boost::asio::generic::raw_protocol;
    using socket_type = typename protocol_type::socket;
    using endpoint_type = typename protocol_type::endpoint;

protected:
    using base_type = netlink_listener;

    ~netlink_listener() = default;

public:
    explicit netlink_listener(boost::asio::io_service & io_service)
        : socket_(io_service) {}

    void open(boost::system::error_code & ec)
    {
        socket_.open(protocol_type(AF_NETLINK, NETLINK_ROUTE), ec);
    }

    void bind(boost::system::error_code & ec)
    {
        struct sockaddr_nl nl;
        std::memset(&nl, 0, sizeof(nl));
        nl.nl_family = AF_NETLINK;
        nl.nl_groups =
              find_group<link_tag>(typename Derived::category())
            | find_group<stats_tag>(typename Derived::category())
            | find_group<ifaddr_v4_tag>(typename Derived::category())
            | find_group<ifaddr_v6_tag>(typename Derived::category())
            | find_group<neighbor_v4_tag>(typename Derived::category())
            | find_group<neighbor_v6_tag>(typename Derived::category());
        socket_.bind(endpoint_type(&nl, sizeof(nl)), ec);
    }

    void start(boost::system::error_code & ec)
    {
        if (!socket_.is_open()) {
            open(ec);
            if (ec) return;
            bind(ec);
            if (ec) return;
        }

        async_receive();
    }

    void start()
    {
        boost::system::error_code ec;
        start(ec);
        boost::asio::detail::throw_error(ec);
    }

    void close(boost::system::error_code & ec)
    {
        socket_.close(ec);
    }

    void close()
    {
        boost::system::error_code ec;
        socket_.close();
        boost::asio::detail::throw_error(ec);
    }

private:
    void async_receive()
    {
        socket_.async_receive(boost::asio::buffer(buffer_), std::bind(&netlink_listener::dispatch, this, std::placeholders::_1, std::placeholders::_2));
    }

    void dispatch(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            on_error(error);
            return;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"

        for(auto * nlmsg = reinterpret_cast<struct ::nlmsghdr *>(buffer_.data());
            NLMSG_OK(nlmsg, size); nlmsg = NLMSG_NEXT(nlmsg, size)) {
            if (nlmsg->nlmsg_type == NLMSG_DONE || nlmsg->nlmsg_type == NLMSG_ERROR) {
                break;
            }

            this->dispatch_link(typename Derived::category(), nlmsg);
            this->dispatch_ifaddr(typename Derived::category(), nlmsg);
            this->dispatch_neighbor(typename Derived::category(), nlmsg);
        }

#pragma GCC diagnostic pop
        async_receive();
    }

    template <typename Tag, typename T>
    static uint find_group(T) { return std::is_base_of<Tag, T>::value ? Tag::group : 0; }

    void on_error(boost::system::error_code const &) {}

private:
    socket_type socket_;
    std::array<char, 4096> buffer_;
};

} } }
