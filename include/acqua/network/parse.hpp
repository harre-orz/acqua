/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>

#include <iterator>
#include <type_traits>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network {

//! Derived ヘッダーにパース
template <
    typename Derived,
    typename Header,
    typename It,
    typename std::enable_if<std::is_base_of<detail::header_base<Header>, Header>::value>::type * = nullptr
    >
ACQUA_DECL Derived * parse(Header * hdr, It & end) noexcept
{
    static_assert(std::is_base_of<detail::header_base<Derived>, Derived>::value, "Derived is base of header_base<Derived>");

    auto * ptr = reinterpret_cast<typename std::iterator_traits<It>::pointer>(hdr);

    if (ptr + sizeof(*hdr) < &(*end)) {
        auto * nxt = reinterpret_cast<Derived *>(ptr + hdr->size());
        if (ptr + nxt->size() < &(*end) && detail::is_match_condition<Header, Derived>()(*hdr, *nxt)) {
            nxt->shrink(end);
            return nxt;
        }
    }
    return nullptr;
}


//! Derived ヘッダーにパース
template <typename Derived, typename It>
ACQUA_DECL Derived * parse(It & beg, It & end) noexcept
{
    static_assert(std::is_base_of<detail::header_base<Derived>, Derived>::value, "Derived is base of header_base<Derived>");
    static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "It was must iterator of 1 byte.");

    if (beg + sizeof(Derived) < end) {
        auto * hdr = reinterpret_cast<Derived *>(&(*beg));
        if (beg + hdr->size() < end) {
            hdr->shrink(end);
            return hdr;
        }
    }
    return nullptr;
}

} }
