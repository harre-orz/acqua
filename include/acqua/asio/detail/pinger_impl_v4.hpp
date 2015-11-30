#pragma once

#include <acqua/asio/socket_category.hpp>
#include <acqua/network/icmp_header.hpp>
#include <acqua/network/parse.hpp>

namespace acqua { namespace asio { namespace detail {

class pinger_impl_v4
{
    using icmp_echo = acqua::network::icmp_echo;

public:
    using category = internet_v4_tag;

    template <typename It>
    static icmp_echo * generate(It beg, It & end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        if ((end - beg) < static_cast<std::ptrdiff_t>(sizeof(icmp_echo)))
            return nullptr;
        auto * echo = reinterpret_cast<icmp_echo *>(&*beg);
        echo->type(icmp_echo::echo_request_message);
        echo->code(0);
        end = beg + echo->header_size();
        return echo;
    }

    template <typename It>
    static icmp_echo const * parse(It beg, It end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        if (auto * ip = acqua::network::parse<acqua::network::ipv4_header const>(beg, end))
            if (auto * echo = acqua::network::parse<icmp_echo>(ip, end))
                if (echo->type() == icmp_echo::echo_reply_message)
                    return echo;
        return nullptr;
    }
};

} } }
