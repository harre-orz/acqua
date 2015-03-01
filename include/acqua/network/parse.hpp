#pragma once

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
inline Derived * parse(Header * hdr, It & end) noexcept
{
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
inline Derived * parse(It & beg, It & end) noexcept
{
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
