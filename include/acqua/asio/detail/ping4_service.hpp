#pragma once

#include <acqua/network/icmp_header.hpp>
#include <acqua/network/parse.hpp>

namespace acqua { namespace asio { namespace detail {

class ping4_service
    : boost::noncopyable
{
public:
    using protocol_type = boost::asio::ip::icmp;

    static protocol_type protocol() noexcept
    {
        return protocol_type::v4();
    }

    static bool is_valid_address(typename protocol_type::endpoint const & ep)
    {
        return ep.address().is_v4();
    }

    template <typename It>
    static acqua::network::icmp_echo * make_icmp_echo(It beg, It & end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        acqua::network::icmp_echo * echo = reinterpret_cast<acqua::network::icmp_echo *>(&*beg);
        echo->type(acqua::network::icmp_header::echo_request_message);
        echo->code(0);
        end = beg + echo->size();
        return echo;
    }

    template <typename It>
    static acqua::network::icmp_echo const * parse_icmp_echo(It beg, It end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        if (auto * ip = acqua::network::parse<acqua::network::ipv4_header>(beg, end))
            if (auto * echo = acqua::network::parse<acqua::network::icmp_echo>(ip, end))
                if (echo->type() == acqua::network::icmp_header::echo_reply_message)
                    return echo;
        return nullptr;
    }

};

} } }
