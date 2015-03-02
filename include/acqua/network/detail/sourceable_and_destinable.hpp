/*!
  The acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <cstdint>

namespace acqua { namespace network { namespace detail {

template <typename Derived, typename Address, typename Class, typename Type, Type Class::* PtrToMember1, Type Class::* PtrToMember2>
class sourceable_and_destinable
{
protected:
    ~sourceable_and_destinable() = default;

public:
    Address const & source() const noexcept
    {
        return reinterpret_cast<Address const &>((static_cast<Class const *>(static_cast<Derived const *>(this)))->*PtrToMember1);
    }

    void source(Address const & addr) noexcept
    {
        reinterpret_cast<Address &>((static_cast<Class *>(static_cast<Derived *>(this)))->*PtrToMember1) = addr;
    }

    Address const & destinate() const noexcept
    {
        return reinterpret_cast<Address const &>((static_cast<Class const *>(static_cast<Derived const *>(this)))->*PtrToMember2);
    }

    void destinate(Address const & addr) noexcept
    {
        reinterpret_cast<Address &>((static_cast<Class *>(static_cast<Derived *>(this)))->*PtrToMember2) = addr;
    }
};


template <typename Derived, typename Class, std::uint16_t Class::* PtrToMember1, std::uint16_t Class::* PtrToMember2>
struct sourceable_and_destinable<Derived, std::uint16_t, Class, std::uint16_t, PtrToMember1, PtrToMember2>
{
protected:
    ~sourceable_and_destinable() = default;

public:
    std::uint16_t source() const noexcept
    {
        return ntohs(static_cast<Class const *>(static_cast<Derived const *>(this))->*PtrToMember1);
    }

    void source(std::uint16_t port) noexcept
    {
        static_cast<Class *>(static_cast<Derived *>(this))->*PtrToMember1 = htons(port);
    }

    std::uint16_t destinate() const noexcept
    {
        return ntohs(static_cast<Class const *>(static_cast<Derived const *>(this))->*PtrToMember2);
    }

    void destinate(std::uint16_t port) noexcept
    {
        static_cast<Class *>(static_cast<Derived *>(this))->*PtrToMember2 = htons(port);
    }
};

} } }
