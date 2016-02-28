/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <type_traits>
#include <acqua/network/detail/header_base.hpp>
#include <acqua/network/detail/is_match_condition.hpp>

namespace acqua { namespace network { namespace detail {

template <typename T, typename Enabler = void>
struct is_parseable_header
{
    static const bool value = false;
};

template <typename T>
struct is_parseable_header<
    T,
    typename std::enable_if<
        std::is_base_of<
            header_base<typename std::remove_const<T>::type>, T
            >::value
        >::type>
{
    static const bool value = true;
};

/*!
  ミュータブルな beg から end までのバッファを Hdr 型にキャストする.
 */
template <typename Hdr, typename It,
          typename std::enable_if<!std::is_const< typename std::remove_pointer< typename std::iterator_traits<It>::pointer >::type>::value>::type * = nullptr>
inline Hdr * parse(It beg, It & end) noexcept
{
    static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    static_assert(is_parseable_header<Hdr>::value, "Hdr is base of header_base");

    if (beg + sizeof(Hdr) < end) {
        auto * hdr = reinterpret_cast<Hdr *>(&(*beg));
        if (beg + hdr->header_size() < end) {
            hdr->shrink_into_end(end);
            return hdr;
        }
    }
    return nullptr;
}


/*!
  イミュータブルな beg から end までのバッファを Hdr const型にキャストする.
 */
template <typename Hdr, typename It,
          typename std::enable_if<std::is_const< typename std::remove_pointer< typename std::iterator_traits<It>::pointer >::type>::value>::type * = nullptr>
inline typename std::remove_const<Hdr>::type const * parse(It beg, It & end) noexcept
{
    static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    static_assert(is_parseable_header<Hdr>::value, "Hdr is base of header_base");

    if (beg + sizeof(Hdr) < end) {
        auto * hdr = reinterpret_cast<typename std::remove_const<Hdr>::type const *>(&(*beg));
        if (beg + hdr->header_size() < end) {
            hdr->shrink_into_end(end);
            return hdr;
        }
    }
    return nullptr;
}


/*!
  ミュータブルな hdr と end から、hdr の次のヘッダー Hdr 型を取得する.
  次のヘッダーが Hdr でない場合は nullptr が返る
  T から Hdr への変換が不明な場合は、コンパイルエラーになる
 */
template <typename Hdr, typename T, typename It,
          typename std::enable_if<is_parseable_header<T>::value>::type * = nullptr>
inline Hdr * parse(T * hdr, It & end) noexcept
{
    static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    static_assert(is_parseable_header<Hdr>::value, "Hdr is base of header_base");

    auto * beg = reinterpret_cast<typename std::iterator_traits<It>::pointer>(hdr);
    if (beg + hdr->header_size() < &(*end)) {
        auto * nxt = reinterpret_cast<Hdr *>(beg + hdr->header_size());
        if (beg + nxt->header_size() < &(*end) &&
            is_match_condition<typename std::remove_const<T>::type, typename std::remove_const<Hdr>::type>()(*hdr, *nxt)) {
            nxt->shrink_into_end(end);
            return nxt;
        }
    }

    return nullptr;
}


/*!
  イミュータブルな hdr と end から、hdr の次のヘッダー Hdr const 型を取得する.
  次のヘッダーが Hdr でない場合は nullptr が返る
  T から Hdr への変換が不明な場合は、コンパイルエラーになる
 */
template <typename Hdr, typename T, typename It,
          typename std::enable_if<is_parseable_header<T>::value>::type * = nullptr>
inline typename std::remove_const<Hdr>::type const * parse(T const * hdr, It & end) noexcept
{
    static_assert(sizeof(typename std::iterator_traits<It>::value_type) == 1, "");
    static_assert(is_parseable_header<Hdr>::value, "Hdr is base of header_base");

    auto const * beg = reinterpret_cast<typename std::iterator_traits<It>::pointer>(const_cast<T *>(hdr));
    if (beg + hdr->header_size() < &(*end)) {
        auto * nxt = reinterpret_cast<Hdr const *>(beg + hdr->header_size());
        if (beg + nxt->header_size() < &(*end) &&
            is_match_condition<typename std::remove_const<T>::type, typename std::remove_const<Hdr>::type>()(*hdr, *nxt)) {
            nxt->shrink_into_end(end);
            return nxt;
        }
    }

    return nullptr;
}

} // detail

using detail::parse;

} }
