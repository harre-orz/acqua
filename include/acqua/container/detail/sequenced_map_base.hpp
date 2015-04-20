/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <algorithm>
#include <functional>
#include <boost/iterator_adaptors.hpp>

namespace acqua { namespace container { namespace detail {

template <typename Tag, typename Key, typename Value, typename Pred, typename Allocator, template <typename V, typename A> class Container>
struct sequenced_map_base
    : private Pred, private Allocator
{
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<key_type, mapped_type>;
    using key_equal = Pred;
    using allocator_type = Allocator;
    using data_type = Container<value_type, allocator_type>;
    using difference_type = typename std::iterator_traits<typename data_type::iterator>::difference_type;
    using size_type = typename data_type::size_type;

    struct iterator : boost::iterator_adaptor<iterator, typename data_type::iterator>
    {
        iterator(typename data_type::iterator it)
            : boost::iterator_adaptor<iterator, typename data_type::iterator>(it) {}
    };

    struct const_iterator : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator>
    {
        const_iterator(typename data_type::const_iterator it)
            : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator>(it) {}

        const_iterator(iterator it)
            : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator>(it.base()) {}
    };

    struct reverse_iterator : boost::iterator_adaptor<reverse_iterator, typename data_type::reverse_iterator>
    {
        reverse_iterator(typename data_type::reverse_iterator it)
            : boost::iterator_adaptor<reverse_iterator, typename data_type::reverse_iterator>(it) {}
    };

    struct const_reverse_iterator : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator>
    {
        const_reverse_iterator(typename data_type::const_reverse_iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator>(it) {}

        const_reverse_iterator(iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator>(it.base()) {}
    };

    sequenced_map_base() = default;

    explicit sequenced_map_base(key_equal pred, allocator_type alloc)
        : data_(pred, alloc) {}

    explicit sequenced_map_base(size_type size)
        : data_(size) {}

    explicit sequenced_map_base(const_iterator beg, const_iterator end)
        : data_(std::distance(beg, end))
    {
        std::copy(beg.base(), end.base(), data_.begin());
    }

    template <typename Iterator>
    explicit sequenced_map_base(Iterator beg, Iterator end)
    {
        // TODO
    }

    key_equal & key_eq()
    { return *static_cast<key_equal *>(this); }

    key_equal const & key_eq() const
    { return *static_cast<key_equal const *>(this); }

    allocator_type & get_allocator()
    { return *static_cast<allocator_type *>(this); }

    allocator_type const & get_allocator() const
    { return *static_cast<allocator_type const *>(this); }

    bool empty() const
    { return data_.empty(); }

    size_type size() const
    { return data_.size(); }

    void clear()
    { data_.clear(); }

    iterator begin()
    { return data_.begin(); }

    iterator end()
    { return data_.end(); }

    const_iterator begin() const
    { return data_.begin(); }

    const_iterator end() const
    { return data_.end(); }

    reverse_iterator rbegin()
    { return data_.rbegin(); }

    reverse_iterator rend()
    { return data_.rend(); }

    const_reverse_iterator rbegin() const
    { return data_.rbegin(); }

    const_reverse_iterator rend() const
    { return data_.rend(); }

    iterator find(key_type const & key)
    { return std::find_if(begin(), end(), [this, &key](value_type const & e) { return key_eq()(e.first, key); }); }

    const_iterator find(key_type const & key) const
    { return std::find_if(begin(), end(), [this, &key](value_type const & e) { return key_eq()(e.first, key); }); }

    template <typename Iterator>
    Iterator to_next(unique_tag, Iterator it) const
    {
        if (it != end())
            ++it;
        return it;
    }

    template <typename Iterator>
    Iterator to_next(non_unique_tag, Iterator it) const
    {
        if (it != end()) {
            auto const & key = it->key;
            do ++it; while(it != end() && key_eq(key, it->key));
        }
        return it;
    }

    size_type count(key_type const & key) const
    {
        auto it = find(key);
        return to_next(Tag(), it) - it;
    }

    std::pair<iterator, iterator> equal_range(key_type const & key)
    {
        auto it = find(key);
        return std::make_pair(it, to_next(Tag(), it));
    }

    std::pair<const_iterator, const_iterator> equal_range(key_type const & key) const
    {
        auto it = find(key);
        return std::make_pair(it, to_next(Tag(), it));
    }

    std::pair<iterator, bool> insert(value_type const & val)
    {
        return insert_(Tag(), val);
    }

    std::pair<iterator, bool> insert_(unique_tag, value_type const & val)
    {
        auto it = find(val.first);
        if (it != end())
            return std::make_pair(it, false);
        data_.push_back(val);
        return std::make_pair(--end(), true);
    }

    std::pair<iterator, bool> insert_(non_unique_tag, value_type const & val)
    {
        return std::make_pair(iterator(data_.insert(find(key(val.first)).base(), val)), true);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args... args)
    {
        return emplace_(Tag(), args...);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_(unique_tag, Args... args)
    {
        auto it = find(key(args...));
        if (it != end())
            return std::make_pair(it, false);
        data_.emplace_back(args...);
        return std::make_pair(--end(), true);
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace_(non_unique_tag, Args... args)
    {
        return std::make_pair(iterator(data_.emplace(find(key(args...)).base(), args...)), true);
    }

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args... args)
    {
        return emplace(args...).first;
    }

    iterator erase(const_iterator it)
    {
        return data_.erase(it.base());
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        return data_.erase(beg.base(), end.base());
    }

    iterator erase(key_type const & key)
    {
        auto it = find(key);
        return data_.erase(it.base(), to_next(Tag(), it).base());
    }

    mapped_type & at(key_type const & key)
    {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("sequenced_map_base::at");
        return it->second;
    }

    mapped_type const & at(key_type const & key) const
    {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("sequenced_map_base::at");
        return it->second;
    }

    mapped_type & operator[](key_type const & key)
    {
        auto it = find(key);
        if (it == end()) {
            data_.emplace_back(key, mapped_type());
            it = --end();
        }
        return it->second;
    }

    static key_type const & key(value_type const & val)
    {
        return val.first;
    }

    template <typename T, typename... Args>
    static T const & key(T const & t, Args...)
    {
        return t;
    }

    data_type data_;
};

} } }
