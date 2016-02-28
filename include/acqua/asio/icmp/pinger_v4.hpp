#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/asio/icmp/basic_pinger.hpp>
#include <acqua/network/icmp_header.hpp>
#include <acqua/network/parse.hpp>

namespace acqua { namespace asio { namespace icmp {

namespace detail {

class pinger_v4_base
{
public:
    void open(boost::asio::ip::icmp::socket & socket, boost::system::error_code & ec) const
    {
        socket.open(boost::asio::ip::icmp::v4(), ec);
        if (ec) return;
        socket.bind(boost::asio::ip::icmp::endpoint(boost::asio::ip::address_v4::any(), 0), ec);
    }

    bool check(boost::asio::ip::icmp::endpoint const & endpoint) const
    {
        return endpoint.address().is_v4();
    }

    template <typename It>
    acqua::network::icmp_echo * generate(It beg, It end) const
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        auto * echo = reinterpret_cast<acqua::network::icmp_echo *>(&*beg);
        if ((end - beg) < static_cast<std::ptrdiff_t>(sizeof(*echo)))
            return nullptr;

        echo->type(echo->echo_request_message);
        echo->code(0);
        end = beg + echo->header_size();
        return echo;
        return nullptr;
    }

    template <typename It>
    acqua::network::icmp_echo const * parse(It beg, It end) const
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        if (auto * ip = acqua::network::parse<acqua::network::ipv4_header const>(beg, end))
            if (auto * echo = acqua::network::parse<acqua::network::icmp_echo>(ip, end))
                if (echo->type() == echo->echo_reply_message)
                    return echo;
        return nullptr;
    }
};

}

using pinger_v4 = basic_pinger<detail::pinger_v4_base>;

} } }
