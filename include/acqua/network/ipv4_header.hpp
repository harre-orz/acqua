/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

extern "C" {
#include <netinet/ip.h>
}

#include <acqua/network/internet4_address.hpp>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/pseudo_header.hpp>
#include <acqua/network/detail/sourceable_and_destinable.hpp>
#include <acqua/network/detail/checkable.hpp>

namespace acqua { namespace network { namespace detail {

class ipv4_header
    : public header_base<ipv4_header>
    , private ::ip
    , public sourceable_and_destinable<ipv4_header, internet4_address, ::ip, struct in_addr, &::ip::ip_src, &::ip::ip_dst>
    , public checkable<ipv4_header, ::ip, u_short, &::ip::ip_sum, ipv4_checksum >
{
    friend sourceable_and_destinable;
    friend checkable;
    friend pseudo_header<ipv4_header>;
    using value_type = ::ip;

public:
    enum protocol_type {
        icmp = 1,
        tcp = 6,
        udp = 17,
    };

    // IPv4ヘッダーとヘッダーオプションの長さ.
    std::size_t header_size() const;

    // ペイロードの終端 end を IPv4 の total_length に基づき調整する.
    template <typename It>
    void shrink_into_end(It & end) const;

    protocol_type protocol() const noexcept;

    void protocol(protocol_type n) noexcept;

    std::uint8_t version() const noexcept;

    void version(std::uint8_t n) noexcept;

    std::uint16_t header_length() const noexcept;

    void header_length(std::uint16_t n) noexcept;

    std::uint8_t type_of_service() const noexcept;

    void type_of_service(std::uint8_t n) noexcept;

    std::uint16_t total_length() const noexcept;

    void total_length(std::uint16_t n) noexcept;

    std::uint16_t id() const noexcept;

    void id(std::uint16_t n) noexcept;

    bool is_dont_flagment() const noexcept;

    void set_dont_flagment() noexcept;

    std::uint8_t time_of_live() const noexcept;

    void time_of_live(std::uint8_t n) noexcept;

    friend std::ostream & operator<<(std::ostream & os, ipv4_header const & rhs);
};

}  // detail

using ipv4_header = detail::ipv4_header;

} }

#include <acqua/network/impl/ipv4_header.ipp>
