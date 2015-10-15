/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

extern "C" {
#include <arpa/inet.h>
}

#include <cstdint>
#include <numeric>
#include <acqua/network/detail/pseudo_header.hpp>

namespace acqua { namespace network { namespace detail {

template <typename T, typename U>
inline std::size_t total_checksum(T const * beg, U const * end) noexcept
{
    auto a = reinterpret_cast<std::uint8_t const *>(beg);
    auto b = reinterpret_cast<std::uint8_t const *>(end);

    std::size_t sum = ntohs(((b - a) % 2) ? *--b : 0);
    while(a != b) {
        sum += ntohs(*reinterpret_cast<std::uint16_t const *>(a));
        sum = (sum & 0xffff) + (sum >> 16);
        a += 2;
    }

    return htons(sum);
}


//! IPv4ヘッダー用のチェックサム計算メソッド
template <typename Derived>
class ipv4_checksum
{
protected:
    ~ipv4_checksum() = default;

public:
    bool check_checksum() const noexcept
    {
        auto sum = total_checksum(this, header_end());
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        return (sum == 0xffff);
    }

    void compute_checksum() noexcept
    {
        (static_cast<Derived *>(this))->checksum(0);
        auto sum = total_checksum(this, header_end());
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        (static_cast<Derived *>(this))->checksum(~sum);
    }

private:
    std::uint8_t const * header_end() const noexcept
    {
        return reinterpret_cast<std::uint8_t const *>(this) + static_cast<Derived const *>(this)->size();
    }
};


//! ICMP, ICMPv6 のためのチェックサム計算メソッド
template <typename Derived>
class data_checksum
{
protected:
    ~data_checksum() = default;

public:
    template <typename It>
    bool check_checksum(It const & end) const noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        auto sum = total_checksum(this, end);
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        return (sum == 0xffff);
    }

    template <typename It>
    void compute_checksum(It const & end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        (static_cast<Derived *>(this))->checksum(0);
        auto sum = total_checksum(this, end);
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        (static_cast<Derived *>(this))->checksum(~sum);
    }
};


//! TCP, UDPのためのチェックサム計算メソッド
template <typename Derived>
class header_and_data_checksum
{
protected:
    ~header_and_data_checksum() = default;

public:
    template <typename Hdr, typename It>
    bool check_checksum(Hdr const * hdr, It const & end) const noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        auto sum = total_checksum(this, end);
        pseudo_header<Hdr>(hdr, distance(end)).merge_checksum(sum);
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        return (sum == 0xffff);
    }

    template <typename Hdr, typename It>
    void compute_checksum(Hdr const * hdr, It const & end) noexcept
    {
        static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");

        (static_cast<Derived *>(this))->checksum(0);
        auto sum = total_checksum(this, end);
        pseudo_header<Hdr>(hdr, distance(end)).checksum(sum);
        sum = (sum & 0xffff) + (sum >> 16);
        sum = (sum & 0xffff) + (sum >> 16);
        (static_cast<Derived *>(this))->checksum(~sum);
    }

private:
    template <typename Ch>
    std::size_t distance(Ch const * end) const noexcept
    {
        return reinterpret_cast<std::uint8_t const *>(end) - reinterpret_cast<std::uint8_t const *>(this);
    }
};


template <typename Derived, typename Class, typename Type, Type Class::* PtrToMember, template <typename T> class Method>
class checkable : public Method<Derived>
{
    friend class Method<Derived>;

protected:
    ~checkable() = default;

public:
    std::size_t checksum() const noexcept
    {
        return static_cast<Class const *>(static_cast<Derived const *>(this))->*PtrToMember;
    }

    void checksum(std::size_t sum) noexcept
    {
        static_cast<Class *>(static_cast<Derived *>(this))->*PtrToMember = sum;
    }

};

} } }
