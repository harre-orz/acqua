#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/container/detail/sequenced_map_base.hpp>
#include <boost/operators.hpp>
#include <deque>
#include <list>

namespace acqua { namespace container {

/*!
  挿入順のマップ.

  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Alloc = std::allocator< std::pair<Key const, Value> >
    >
class sequenced_map
    : public detail::sequenced_map_base<sequenced_map<Key, Value, Pred, Alloc>, Key, Value, Pred, Alloc, std::deque>
    , private boost::totally_ordered< sequenced_map<Key, Value, Pred, Alloc> >
{
private:
    using base_type = typename sequenced_map::base_type;
    using base_type::data_;

public:
    using value_type = typename base_type::value_type;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;

    using base_type::begin;
    using base_type::end;
    using base_type::find;

public:
    sequenced_map(Pred const & key_eq = Pred(), Alloc const & alloc = Alloc())
        : base_type(key_eq, alloc) {}

    template <typename It>
    explicit sequenced_map(It beg, It end)
        : base_type(Pred(), Alloc())
    {
        base_type::insert(beg, end);
    }

    iterator insert(const_iterator, value_type const & val)
    {
        auto it = find(val.first);
        if (it != end())
            return it;
        data_.push_back(val);
        return --end();
    }

    std::pair<iterator, bool> insert(value_type const & val)
    {
        auto it = find(val.first);
        if (it != end())
            return std::make_pair(it, false);
        data_.push_back(val);
        return std::make_pair(--end(), true);
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args... args)
    {
        auto it = find(args...);
        if (it != end())
            return it;
        data_.emplace_back(args...);
        return --end();
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        auto it = find(args...);
        if (it != end())
            return std::make_pair(it, false);
        data_.emplace_back(args...);
        return std::make_pair(--end(), true);
    }

    Value & operator[](Key const & key)
    {
        auto it = base_type::find(key);
        if (it == base_type::end()) {
            data_.emplace_back(key, Value());
            it = --base_type::end();
        }
        return it->second;
    }

    friend bool operator==(sequenced_map const & lhs, sequenced_map const & rhs)
    {
        return lhs.data_ == rhs.data_;
    }

    friend bool operator<(sequenced_map const & lhs, sequenced_map const & rhs)
    {
        return lhs.data_ < rhs.data_;
    }
};


/*!
  挿入順のマルチマップ.

  同一のキーが存在する場合は、必ず既存のデータの後にデータを追加する
  検索や終端でない位置に挿入する速度は、データ数に応じて線形的に遅くなるので、大きいデータを扱うのには不向き
 */
template <
    typename Key,
    typename Value,
    typename Pred = std::equal_to<Key>,
    typename Alloc = std::allocator< std::pair<Key const, Value> >
    >
class sequenced_multimap
    : public acqua::container::detail::sequenced_map_base< sequenced_multimap<Key, Value, Pred, Alloc>, Key, Value, Pred, Alloc, std::deque>
    , private boost::totally_ordered< sequenced_multimap<Key, Value, Pred, Alloc> >
{
private:
    using base_type = typename sequenced_multimap::base_type;
    using base_type::data_;

public:
    using value_type = typename base_type::value_type;
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;

    using base_type::begin;
    using base_type::end;
    using base_type::find;

public:
    sequenced_multimap(Pred const & key_eq = Pred(), Alloc const & alloc = Alloc())
        : base_type(key_eq, alloc) {}

    template <typename It>
    explicit sequenced_multimap(It beg, It end)
        : base_type(Pred(), Alloc())
    {
        base_type::insert(beg, end);
    }

    iterator insert(const_iterator, value_type const & val)
    {
        auto it = find(val.first);
        if (it != end()) {
            advance_to_next_key(it);
            return data_.insert(it.base(), val);
        }
        data_.push_back(val);
        return --end();
    }

    std::pair<iterator, bool> insert(value_type const & val)
    {
        return std::make_pair(insert(const_iterator(), val), true);
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args... args)
    {
        auto it = find(args...);
        if (it != end()) {
            advance_to_next_key(it);
            return data_.insert(it.base(), value_type(args...));
        }
        data_.emplace_back(args...);
        return --end();
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        return std::make_pair(emplace_hint(const_iterator(), args...), true);
    }

    friend bool operator==(sequenced_multimap const & lhs, sequenced_multimap const & rhs)
    {
        return lhs.data_ == rhs.data_;
    }

    friend bool operator<(sequenced_multimap const & lhs, sequenced_multimap const & rhs)
    {
        return lhs.data_ < rhs.data_;
    }

private:
    template <typename It>
    void advance_to_next_key(It & it) const
    {
        for(auto const & key = it->first; ++it != end();)
            if (!Pred()(key, it->first))
                break;
    }
};

} }
