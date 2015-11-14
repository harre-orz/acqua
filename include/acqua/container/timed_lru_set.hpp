/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <chrono>
#include <acqua/container/detail/lru_cache_facade.hpp>
#include <acqua/container/detail/timed_lru_set_impl.hpp>

namespace acqua { namespace container {

/*!
 * 時間制限付き Least Recently Used のアルゴリズムを用いたセット.
 *
 * 検索は高速なハッシュで行いつつ、データ数が一定数を超えたり有効時間が過ぎた最も古いデータが削除されるデータ構造
 */
template <
    typename T,
    typename Hash = std::hash<T>,
    typename Pred = std::equal_to<T>,
    typename Alloc = std::allocator<T>,
    typename Clock = std::chrono::steady_clock
    >
class timed_lru_set
    : private detail::lru_cache_facade< detail::timed_lru_set_impl<T, Hash, Pred, Alloc, Clock> >
{
    using base_type = typename timed_lru_set::base_type;
    using impl_type = typename timed_lru_set::impl_type;

public:
    using size_type = typename impl_type::size_type;
    using value_type = typename impl_type::value_type;
    using allocator_type = typename impl_type::allocator_type;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;
    using clock_type = typename impl_type::clock_type;
    using time_point_type = typename impl_type::time_point_type;
    using duration_type = typename impl_type::duration_type;

public:
    timed_lru_set()
        : base_type(allocator_type(), 32)
    {
    }

    timed_lru_set(timed_lru_set const & rhs) = default;

    timed_lru_set(timed_lru_set && rhs) = default;

    timed_lru_set & operator=(timed_lru_set const & rhs) = default;

    timed_lru_set & operator=(timed_lru_set && rhs) = default;

    using base_type::get_allocator;
    using base_type::empty;
    using base_type::size;
    using base_type::begin;
    using base_type::end;
    using base_type::rbegin;
    using base_type::rend;
    using base_type::cbegin;
    using base_type::cend;
    using base_type::crbegin;
    using base_type::crend;
    using base_type::front;
    using base_type::back;
    using base_type::find;
    using base_type::erase;
    using base_type::push;
    using base_type::pop;
    using base_type::insert;
    using base_type::emplace;
    using base_type::emplace_hint;
    using base_type::shrink_to_fit;
    using base_type::node_element_size;
    using base_type::bucket_count;

    size_type max_size() const
    {
        return impl_type::get_max_size();
    }

    void max_size(size_type size)
    {
        impl_type::set_max_size(size);
    }

    duration_type expire() const
    {
        return impl_type::get_expire();
    }

    void expire(duration_type const & duration) const
    {
        impl_type::set_expire(duration);
    }
};

} }
