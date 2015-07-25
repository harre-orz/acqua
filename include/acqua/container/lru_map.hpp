/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/container/detail/lru_cache.hpp>

namespace acqua { namespace container {

template <
    typename K,
    typename V,
    typename Hash = std::hash<K>,
    typename Pred = std::equal_to<K>,
    typename Allocator = std::allocator< std::pair<K const, V> >
    >
class lru_map
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<K const, V>;
    using hasher = Hash;
    using key_equal = Pred;
    using allocator_type = Allocator;
    using size_type = std::size_t;

private:
    struct node
    {
        value_type value_;

        node(value_type const & v)
            : value_(v) {}

        node & operator=(value_type const & rhs)
        {
            const_cast<key_type &>(value_.first) = rhs.first;
            value_.second = rhs.second;
            return *this;
        }

        node & operator=(value_type && rhs)
        {
            const_cast<key_type &>(value_.first) = std::move(rhs.first);
            value_.second = std::move(rhs.second);
            return *this;
        }

        friend bool operator==(node const & lhs, node const & rhs)
        {
            return key_equal()(lhs.value_.first, rhs.value_.first);
        }

        friend size_type hash_value(node const & rhs)
        {
            return hasher()(rhs.value_.first);
        }
    };

    struct key_node_equal
    {
        bool operator()(key_type const & lhs, node const & rhs) const
        {
            return key_equal()(lhs, rhs.value_.first);
        }
    };

    using base_type = detail::lru_cache<node, value_type, allocator_type>;

public:
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;

    lru_map(size_type bucket_size, allocator_type alloc = allocator_type())
        : base_(bucket_size, alloc) {}

    void clear() { base_.clear(); }
    bool empty() const { return base_.empty(); }
    size_type size() const { return base_.size(); }
    iterator begin() { return base_.begin(); }
    const_iterator begin() const { return base_.begin(); }
    iterator end() { return base_.end(); }
    const_iterator end() const { return base_.end(); }
    reverse_iterator rbegin() { return base_.rbegin(); }
    const_reverse_iterator rbegin() const { return base_.rbegin(); }
    reverse_iterator rend() { return base_.rend(); }
    const_reverse_iterator rend() const { return base_.rend(); }
    void push(value_type const & v) { base_.push(v); }
    void push(value_type && v) { base_.push(std::move(v)); }
    void pop() { base_.pop(); }
    value_type & front() { return base_.front(); }
    value_type const & front() const { return base_.front(); }
    value_type & back() { return base_.back(); }
    value_type const & back() const { return base_.back(); }
    iterator find(key_type const & k) { return base_.find(k, hasher(), key_node_equal()); }
    const_iterator find(key_equal const & k) const { return base_.find(k, hasher(), key_node_equal()); }
    iterator erase(const_iterator it) { return base_.erase(it); }
    iterator erase(const_iterator beg, const_iterator end) { return base_.erase(beg, end); }
    size_type max_size() const { return base_.max_size(); }
    void max_size(size_type size) { base_.max_size(size); }
private:
    base_type base_;
};

} }
