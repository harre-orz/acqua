/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>
#include <acqua/container/detail/lru_cache_facade.hpp>
#include <acqua/container/detail/lru_map_impl.hpp>

namespace acqua { namespace container {

template <
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    typename Pred = std::equal_to<K>,
    typename Alloc = std::allocator< std::pair<K const, V> >
    >
class lru_map
    : private detail::lru_cache_facade< detail::lru_map_impl<K, V, Hash, Pred, Alloc> >
{
    using base_type = typename lru_map::base_type;
    using impl_type = typename lru_map::impl_type;

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

public:
    ACQUA_DECL lru_map()
        : base_type(allocator_type(), 32)
    {
    }

    ACQUA_DECL lru_map(lru_map const & rhs)  = default;

    ACQUA_DECL lru_map(lru_map && rhs) = default;

    ACQUA_DECL lru_map & operator=(lru_map const & rhs) = default;

    ACQUA_DECL lru_map & operator=(lru_map && rhs) = default;

    ACQUA_DECL allocator_type get_allocator() const
    {
        return base_type::get_allocator();
    }

    ACQUA_DECL bool empty() const
    {
        return base_type::empty();
    }

    ACQUA_DECL size_type size() const
    {
        return base_type::size();
    }

    ACQUA_DECL iterator begin()
    {
        return base_type::begin();
    }

    ACQUA_DECL const_iterator begin() const
    {
        return base_type::begin();
    }

    ACQUA_DECL iterator end()
    {
        return base_type::end();
    }

    ACQUA_DECL const_iterator end() const
    {
        return base_type::end();
    }

    ACQUA_DECL reverse_iterator rbegin()
    {
        return base_type::rbegin();
    }

    ACQUA_DECL const_reverse_iterator rbegin() const
    {
        return base_type::rbegin();
    }

    ACQUA_DECL reverse_iterator rend()
    {
        return base_type::rend();
    }

    ACQUA_DECL const_reverse_iterator rend() const
    {
        return base_type::rend();
    }

    ACQUA_DECL value_type & front()
    {
        return base_type::front();
    }

    ACQUA_DECL value_type const & front() const
    {
        return base_type::front();
    }

    ACQUA_DECL value_type & back()
    {
        return base_type::back();
    }

    ACQUA_DECL value_type const & back() const
    {
        return base_type::back();
    }

    ACQUA_DECL iterator find(key_type const & key)
    {
        return base_type::find(key);
    }

    ACQUA_DECL const_iterator find(key_type const & key) const
    {
        return base_type::find(key);
    }

    ACQUA_DECL iterator erase(iterator it)
    {
        return base_type::erase(it);
    }

    ACQUA_DECL iterator erase(const_iterator it)
    {
        return base_type::erase(it);
    }

    ACQUA_DECL iterator erase(iterator beg, iterator end)
    {
        return base_type::erase(beg, end);
    }

    ACQUA_DECL iterator erase(const_iterator beg, const_iterator end)
    {
        return base_type::erase(beg, end);
    }

    ACQUA_DECL iterator erase(key_type const & key)
    {
        return base_type::erase(base_type::find(key));
    }

    ACQUA_DECL bool push(value_type const & val)
    {
        return base_type::push(val);
    }

    ACQUA_DECL bool push(value_type && val)
    {
        return base_type::push(std::move(val));
    }

    ACQUA_DECL void pop()
    {
        base_type::pop();
    }

    ACQUA_DECL std::pair<iterator, bool> insert(value_type const & val)
    {
        bool res = push(val);
        return std::make_pair(begin(), res);
    }

    ACQUA_DECL iterator insert(const_iterator, value_type const & val)
    {
        push(val);
        return begin();
    }

    template <typename ... Args>
    ACQUA_DECL std::pair<iterator, bool> emplace(Args... args)
    {
        bool res = push(std::make_pair(args...));
        return std::make_pair(begin(), res);
    }

    template <typename ... Args>
    ACQUA_DECL iterator emplace_hint(const_iterator, Args... args)
    {
        push(std::make_pair(args...));
        return begin();
    }

    ACQUA_DECL size_type max_size() const
    {
        return base_type::get_max_size();
    }

    ACQUA_DECL void max_size(size_type size)
    {
        base_type::set_max_size(size);
    }

    ACQUA_DECL size_type node_element_size() const
    {
        return base_type::node_element_size();
    }
};

} }
