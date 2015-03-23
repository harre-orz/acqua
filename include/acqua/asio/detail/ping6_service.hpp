/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/network/icmpv6_header.hpp>

namespace acqua { namespace asio { namespace detail {

class ping6_service
    : boost::noncopyable
{
public:
    using protocol_type = boost::asio::ip::icmp;

    static protocol_type protocol() noexcept
    {
        return protocol_type::v6();
    }

    template <typename It>
    static acqua::network::icmpv6_echo * make_icmp_echo(It beg, It & end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        acqua::network::icmpv6_echo * echo = reinterpret_cast<acqua::network::icmpv6_echo *>(&*beg);
        echo->type(acqua::network::icmpv6_header::echo_request_message);
        echo->code(0);
        end = beg + echo->size();
        return echo;
    }

    template <typename It>
    static acqua::network::icmpv6_echo const * parse_icmp_echo(It beg, It end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        if (auto * echo = acqua::network::parse<acqua::network::icmpv6_echo>(beg, end))
            if (echo->type() == acqua::network::icmpv6_header::echo_reply_message)
                return echo;
        return nullptr;
    }

};

} } }
