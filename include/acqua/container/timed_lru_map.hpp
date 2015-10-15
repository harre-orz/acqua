/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#pragma once

#include <chrono>
#include <acqua/container/detail/lru_cache_facade.hpp>
#include <acqua/container/detail/timed_lru_map_impl.hpp>

namespace acqua { namespace container {

/*!
 * 時間制限付き Least Recently Used のアルゴリズムを用いたマップ.
 *
 * 検索は高速なハッシュで行いつつ、データ数が一定数を超えたり有効時間が過ぎた最も古いデータが削除されるデータ構造
 */
template <
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    typename Pred = std::equal_to<K>,
    typename Alloc = std::allocator< std::pair<K const, V> >,
    typename Clock = std::chrono::steady_clock
    >
class timed_lru_map
    : private detail::lru_cache_facade< detail::timed_lru_map_impl<K, V, Hash, Pred, Alloc, Clock> >
{
    using base_type = typename timed_lru_map::base_type;
    using impl_type = typename timed_lru_map::impl_type;

public:
    using size_type = typename impl_type::size_type;
    using key_type = typename impl_type::key_type;
    using mapped_type = typename impl_type::mapped_type;
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
    timed_lru_map()
        : base_type(allocator_type(), 32)
    {
    }

    timed_lru_map(timed_lru_map const & rhs) = default;

    timed_lru_map(timed_lru_map && rhs) = default;

    timed_lru_map & operator=(timed_lru_map const & rhs) = default;

    timed_lru_map & operator=(timed_lru_map && rhs) = default;

    allocator_type get_allocator() const
    {
        return base_type::get_allocator();
    }

    bool empty() const
    {
        return base_type::empty();
    }

    size_type size() const
    {
        return base_type::size();
    }

    iterator begin()
    {
        return base_type::begin();
    }

    const_iterator begin() const
    {
        return base_type::begin();
    }

    iterator end()
    {
        return base_type::end();
    }

    const_iterator end() const
    {
        return base_type::end();
    }

    reverse_iterator rbegin()
    {
        return base_type::rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return base_type::rbegin();
    }

    reverse_iterator rend()
    {
        return base_type::rend();
    }

    const_reverse_iterator rend() const
    {
        return base_type::rend();
    }

    value_type & front()
    {
        return base_type::front();
    }

    value_type const & front() const
    {
        return base_type::front();
    }

    value_type & back()
    {
        return base_type::back();
    }

    value_type const & back() const
    {
        return base_type::back();
    }

    iterator find(key_type const & key)
    {
        return base_type::find(key);
    }

    const_iterator find(key_type const & key) const
    {
        return base_type::find(key);
    }

    iterator erase(iterator it)
    {
        return base_type::erase(it);
    }

    iterator erase(const_iterator it)
    {
        return base_type::erase(it);
    }

    iterator erase(iterator beg, iterator end)
    {
        return base_type::erase(beg, end);
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        return base_type::erase(beg, end);
    }

    iterator erase(key_type const & key)
    {
        return base_type::erase(base_type::find(key));
    }

    bool push(value_type const & val)
    {
        return base_type::push(val);
    }

    bool push(value_type && val)
    {
        return base_type::push(std::move(val));
    }

    void pop()
    {
        base_type::pop();
    }

    std::pair<iterator, bool> insert(value_type const & val)
    {
        bool res = push(val);
        return std::make_pair(begin(), res);
    }

    iterator insert(const_iterator, value_type const & val)
    {
        push(val);
        return begin();
    }

    template <typename ... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        bool res = push(value_type(args...));
        return std::make_pair(begin(), res);
    }

    template <typename ... Args>
    iterator emplace_hint(const_iterator, Args... args)
    {
        push(value_type(args...));
        return begin();
    }

    void shrink_to_fit()
    {
        base_type::shrink_to_fit();
    }

    size_type max_size() const
    {
        return impl_type::get_max_size();
    }

    void max_size(size_type size)
    {
        impl_type::set_max_size(size);
        shrink_to_fit();
    }

    duration_type expire() const
    {
        return impl_type::get_expire();
    }

    void expire(duration_type const & duration)
    {
        impl_type::set_expire(duration);
        shrink_to_fit();
    }

    size_type node_element_size() const
    {
        return base_type::node_element_size();
    }
};

} }
