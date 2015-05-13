/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <deque>
#include <list>
#include <acqua/container/detail/container_tag.hpp>
#include <acqua/container/detail/sequenced_map_base.hpp>

namespace acqua { namespace container {

/*!
  挿入順のマップ.

  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Allocator = std::allocator< std::pair<Key const, Value> >
    >
class sequenced_map
    : detail::sequenced_map_base<detail::unique_tag, Key, Value, Pred, Allocator, std::deque>
{
    using base_type = detail::sequenced_map_base<detail::unique_tag, Key, Value, Pred, Allocator, std::deque>;

public:
    using key_type = typename base_type::key_type;
    using mapped_type = typename base_type::mapped_type;
    using value_type = typename base_type::value_type;
    using key_equal = typename base_type::key_equal;;
    using allocator_type = typename base_type::allocator_type;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;
    using difference_type = typename base_type::difference_type;
    using size_type = typename base_type::size_type;

    using base_type::base_type;
    using base_type::key_eq;
    using base_type::get_allocator;
    using base_type::empty;
    using base_type::size;
    using base_type::clear;
    using base_type::begin;
    using base_type::end;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::find;
    using base_type::count;
    using base_type::equal_range;
    using base_type::insert;
    using base_type::emplace;
    using base_type::emplace_hint;
    using base_type::erase;
    using base_type::at;
    using base_type::operator[];
};


/*!
  挿入順のマルチマップ.

  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Allocator = std::allocator< std::pair<Key const, Value> >
    >
class sequenced_multimap
    : public detail::sequenced_map_base<detail::non_unique_tag, Key, Value, Pred, Allocator, std::deque>
{
    using base_type = detail::sequenced_map_base<detail::non_unique_tag, Key, Value, Pred, Allocator, std::deque>;

public:
    using key_type = typename base_type::key_type;
    using mapped_type = typename base_type::mapped_type;
    using value_type = typename base_type::value_type;
    using key_equal = typename base_type::key_equal;;
    using allocator_type = typename base_type::allocator_type;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;
    using difference_type = typename base_type::difference_type;
    using size_type = typename base_type::size_type;

    using base_type::base_type;
    using base_type::key_eq;
    using base_type::get_allocator;
    using base_type::empty;
    using base_type::size;
    using base_type::clear;
    using base_type::begin;
    using base_type::end;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::find;
    using base_type::count;
    using base_type::equal_range;
    using base_type::insert;
    using base_type::emplace;
    using base_type::emplace_hint;
    using base_type::erase;
    using base_type::at;
    using base_type::operator[];
};

} }
