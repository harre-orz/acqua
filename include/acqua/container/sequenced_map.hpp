/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <algorithm>
#include <functional>
#include <deque>
#include <boost/iterator_adaptors.hpp>

namespace acqua { namespace container {

/*!
  挿入順のマップ.

  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Allocator = std::allocator< std::pair<Key const, Value> >,
    template <typename V, typename A> class Container = std::deque
    >
class sequenced_map
{
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key, Value>;
    using key_equal = Pred;
    using allocator_type = Allocator;

private:
    using map_type = Container<value_type, allocator_type>;

public:
    using size_type = typename map_type::size_type;

    class iterator
        : public boost::iterator_adaptor<iterator, typename map_type::iterator>
    {
    public:
        iterator(typename map_type::iterator it)
            : boost::iterator_adaptor<iterator, typename map_type::iterator>(it) {}
    };

    class const_iterator
        : boost::iterator_adaptor<const_iterator, typename map_type::const_iterator>
    {
    public:
        const_iterator(typename map_type::const_iterator it)
            : boost::iterator_adaptor<const_iterator, typename map_type::const_iterator>(it) {}

        const_iterator(iterator it)
            : boost::iterator_adaptor<const_iterator, typename map_type::const_iterator>(it.base()) {}
    };

public:
    sequenced_map() = default;

    sequenced_map(key_equal pred, allocator_type alloc)
        : map_(pred, alloc) {}

    sequenced_map(const_iterator beg, const_iterator end)
    {
        while(beg != end) {
            emplace(*beg);
            ++beg;
        }
    }

    explicit sequenced_map(size_type size)
        : map_(size) {}

    size_type size() const noexcept
    {
        return map_.size();
    }

    bool empty() const noexcept
    {
        return map_.empty();
    }

    void clear()
    {
        return map_.clear();
    }

    iterator begin()
    {
        return iterator(map_.begin());
    }

    const_iterator begin() const
    {
        return const_iterator(map_.begin());
    }

    iterator end()
    {
        return iterator(map_.end());
    }

    const_iterator end() const
    {
        return const_iterator(map_.end());
    }

    iterator erase(const_iterator it)
    {
        return iterator(map_.erase(it.base()));
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        return iterator(map_.erase(beg.base(), end.base()));
    }

    iterator find(key_type const & key)
    {
        return std::find_if(begin(), end(), [&key](value_type const & e) { return key_equal()(e.first, key); });
    }

    const_iterator find(key_type const & key) const
    {
        return std::find_if(begin(), end(), [&key](value_type const & e) { return key_equal()(e.first, key); });
    }

    std::pair<iterator, bool> insert(value_type const & value)
    {
        bool is_new;
        auto it = find(value.first);
        if ((is_new = (it == end()))) {
            map_.push_back(value);
            it = --end();
        }
        return std::make_pair(it, is_new);
    }

    iterator insert(const_iterator, value_type const & value)
    {
        auto it = find(value.first);
        if (it == map_.end())
            it = map_.insert(it, value);
        return it;
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        bool is_new;
        auto it = find_args(args...);
        if ((is_new = (it == end()))) {
            map_.emplace_back(args...);
            it = --end();
        }
        return std::make_pair(it, is_new);
    }

    template <typename... Args>
    iterator emplace_hint(Args... args)
    {
        auto it = find_args(args...);
        if (it == end()) {
            map_.emplace_back(args...);
            it = --end();
        }
        return it;
    }

    std::pair<iterator, iterator> equal_range(key_type const & key)
    {
        auto it = find(key);
        return std::make_pair(it, (it != end() ? std::next(it) : it));
    }

    std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const
    {
        auto it = find(key);
        return std::make_pair(it, (it != end() ? std::next(it) : it));
    }

    mapped_type & operator[](key_type const & key)
    {
        auto it = find(key);
        if (it == end())
            it = iterator(map_.emplace(it.base(), value_type(key, mapped_type())));
        return it->second;
    }

private:
    template <typename... Args>
    iterator find_args(key_type const & key, Args...)
    {
        return find(key);
    }

    iterator find_args(value_type const & value)
    {
        return find(value.first);
    }

    template <typename... Args>
    const_iterator find_args(key_type const & key, Args...) const
    {
        return find(key);
    }

    const_iterator find_args(value_type const & value) const
    {
        return find(value.first);
    }

private:
    map_type map_;
};


/*!
  挿入順のマルチマップ.

  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Allocator = std::allocator< std::pair<Key const, Value> >,
    template <typename V, typename A> class Container = std::deque
    >
class sequenced_multimap
{
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key, Value>;
    using key_equal = Pred;
    using allocator_type = Allocator;

private:
    using multimap_type = Container<value_type, allocator_type>;

public:
    using size_type = typename multimap_type::size_type;

    class iterator
        : public boost::iterator_adaptor<iterator, typename multimap_type::iterator>
    {
    public:
        iterator(typename multimap_type::iterator it)
            : boost::iterator_adaptor<iterator, typename multimap_type::iterator>(it) {}
    };

    class const_iterator
        : boost::iterator_adaptor<const_iterator, typename multimap_type::const_iterator>
    {
    public:
        const_iterator(typename multimap_type::const_iterator it)
            : boost::iterator_adaptor<const_iterator, typename multimap_type::const_iterator>(it) {}

        const_iterator(iterator it)
            : boost::iterator_adaptor<const_iterator, typename multimap_type::const_iterator>(it.base()) {}
    };

public:
    sequenced_multimap() = default;

    sequenced_multimap(key_equal pred, allocator_type alloc)
        : multimap_(pred, alloc) {}

    sequenced_multimap(const_iterator beg, const_iterator end)
    {
        while(beg != end) {
            emplace(*beg);
            ++beg;
        }
    }

    explicit sequenced_multimap(size_type size)
        : multimap_(size) {}

    size_type size() const noexcept
    {
        return multimap_.size();
    }

    bool empty() const noexcept
    {
        return multimap_.empty();
    }

    void clear()
    {
        multimap_.clear();
    }

    iterator begin()
    {
        return multimap_.begin();
    }

    const_iterator begin() const
    {
        return multimap_.begin();
    }

    iterator end()
    {
        return multimap_.end();
    }

    const_iterator end() const
    {
        return multimap_.end();
    }

    iterator erase(const_iterator it)
    {
        return multimap_.erase(it.base());
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        return multimap_.erase(beg.base(), end.base());
    }

    iterator find(key_type const & key)
    {
        return std::find_if(begin(), end(), [&key](value_type const & e) { return key_equal()(e.first, key); });
    }

    const_iterator find(key_type const & key) const
    {
        return std::find_if(begin(), end(), [&key](value_type const & e) { return key_equal()(e.first, key); });
    }

    std::pair<iterator, bool> insert(value_type const & value)
    {
        return std::make_pair(insert(begin(), value), true);
    }

    iterator insert(const_iterator, value_type const & value)
    {
        auto it = find(value.first);
        if (it == end()) {
            multimap_.push_back(value);
            it = --end();
        } else {
            equal_advance(++it, value);
            it = multimap_.insert(it.base(), value);
        }
        return it;
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        return std::make_pair(iterator(emplace_hint(begin(), args...)), true);
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args... args)
    {
        auto it = find_args(args...);
        if (it == end()) {
            multimap_.emplace_back(args...);
            it = --end();
        } else {
            equal_advance(++it, args...);
            it = multimap_.emplace(it.base(), args...);
        }
        return it;
    }

    std::pair<iterator, iterator> equal_range(key_type const & key)
    {
        auto from = find(key);
        auto to = from;
        if (from != end()) {
            equal_advance(++to, key);
        }
        return std::make_pair(from, to);
    }

    std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const
    {
        auto from = find(key);
        auto to = from;
        if (from != end())
            equal_advance(++to, key);
        return std::make_pair(from, to);
    }

private:
    template <typename... Args>
    iterator find_args(key_type const & key, Args...)
    {
        return find(key);
    }

    iterator find_args(value_type const & value)
    {
        return find(value.first);
    }

    template <typename... Args>
    void equal_advance(iterator & it, key_type const & key, Args...)
    {
        while(it != end() && key_equal()(it->first, key))
            ++it;
    }

    void equal_advance(iterator & it, value_type const value)
    {
        while(it != end() && key_equal()(it->first, value.first))
            ++it;
    }

private:
    multimap_type multimap_;
};

} }
