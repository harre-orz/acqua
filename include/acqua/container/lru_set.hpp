/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <chrono>
#include <acqua/container/detail/lru_cache.hpp>

namespace acqua { namespace container {

template <
    typename T,
    typename Clock = std::chrono::steady_clock,
    typename Hash = std::hash<T>,
    typename Pred = std::equal_to<T>,
    typename Allocator = std::allocator<T>
    >
class lru_set
{
public:
    using value_type = T;
    using hasher = Hash;
    using key_equal = Pred;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using clock_type = Clock;
    using time_point_type = typename clock_type::time_point;
    using duration_type = typename clock_type::duration;

private:
    struct node
    {
        value_type value_;

        node(value_type const & v)
            : value_(v) {}

        node(value_type && v)
            : value_(std::move(v)) {}

        friend bool operator==(node const & lhs, node const & rhs)
        {
            return key_equal()(lhs.value_, rhs.value_);
        }

        friend size_type hash_value(node const & rhs)
        {
            return hasher()(rhs.value_);
        }
    };

    struct value_node_equal
    {
        bool operator()(value_type const & lhs, node const & rhs) const
        {
            return key_equal()(lhs, rhs.value_);
        }
    };

    using base_type = detail::lru_cache<value_type, node, std::chrono::steady_clock, Hash, value_node_equal, allocator_type>;

public:
    using iterator = typename base_type::iterator;
    using const_iterator = typename base_type::const_iterator;
    using reverse_iterator = typename base_type::reverse_iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;

    lru_set(size_type bucket_size, allocator_type alloc = allocator_type())
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
    void push(value_type const & v) { base_.push(v, Hash(), value_node_equal()); }
    //void push(value_type && v) { base_.push(std::move(v)); }
    void pop() { base_.pop(); }
    value_type & front() { return base_.front(); }
    value_type const & front() const { return base_.front(); }
    value_type & back() { return base_.back(); }
    value_type const & back() const { return base_.back(); }
    iterator find(value_type const & k) { return base_.find(k, Hash(), value_node_equal()); }
    const_iterator find(key_equal const & k) const { return base_.find(k); }
    iterator erase(const_iterator it) { return base_.erase(it); }
    iterator erase(const_iterator beg, const_iterator end) { return base_.erase(beg, end); }

    size_type max_size() const { return base_.max_size(); }
    void max_size(size_type size) { base_.max_size(size); }
    duration_type const & max_duration() const { return base_.max_duration(); }
    void max_duration(duration_type const & dura) { base_.max_duration(dura); }

private:
    base_type base_;
};



} }
