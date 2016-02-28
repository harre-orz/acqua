#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <iostream>

namespace acqua { namespace container { namespace detail {

template <typename Impl>
class lru_cache_facade
    : public Impl
    , private Impl::allocator_type
{
public:
    using base_type = lru_cache_facade;
    using impl_type = Impl;
    using key_type = typename Impl::key_type;
    using value_type = typename Impl::value_type;
    using size_type = typename Impl::size_type;
    using allocator_type = typename Impl::allocator_type;
    using hasher = typename Impl::hasher;
    using equal_to = typename Impl::equal_to;

    struct iterator;
    struct const_iterator;
    struct reverse_iterator;
    struct const_reverse_iterator;
    static const size_type min_bucket_size = 32;

public:
    lru_cache_facade(allocator_type alloc, size_type size);

    lru_cache_facade(lru_cache_facade const & rhs);

    lru_cache_facade(lru_cache_facade && rhs);

    ~lru_cache_facade();

    lru_cache_facade & operator=(lru_cache_facade const & rhs);

    lru_cache_facade & operator=(lru_cache_facade && rhs);

    allocator_type get_allocator() const;

    bool empty() const;

    size_type size() const;

    iterator begin();

    const_iterator begin() const;

    iterator end();

    const_iterator end() const;

    reverse_iterator rbegin();

    const_reverse_iterator rbegin() const;

    reverse_iterator rend();

    const_reverse_iterator rend() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

    const_reverse_iterator crbegin() const;

    const_reverse_iterator crend() const;

    value_type & front();

    value_type const & front() const;

    value_type & back();

    value_type const & back() const;

    void clear();

    iterator find(key_type const & key);

    const_iterator find(key_type const & key) const;

    iterator erase(const_iterator it);

    iterator erase(const_iterator beg, const_iterator end);

    iterator erase(key_type const & key);

    template <typename U>
    bool push(U && val);

    void pop();

    std::pair<iterator, bool> insert(value_type const & val);

    iterator insert(const_iterator, value_type const & val);

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args);

    template <typename... Args>
    iterator emplace_hint(const_iterator, Args&&... args);

    void shrink_to_fit();

    size_type node_element_size() const
    {
        return sizeof(node);
    }

    size_type bucket_count() const
    {
        return hash_.bucket_count();
    }


private:
    using void_pointer = typename std::allocator_traits<allocator_type>::void_pointer;
    using list_hook = boost::intrusive::list_member_hook<
        boost::intrusive::void_pointer<void_pointer> >;
    using hash_hook = boost::intrusive::unordered_set_member_hook<
        boost::intrusive::void_pointer<void_pointer> >;

    class node
        : public impl_type::node_base
    {
        friend lru_cache_facade;

    private:
        list_hook list_;
        hash_hook hash_;

    public:
        explicit node(value_type const & val) : impl_type::node_base(val) {}
        explicit node(value_type && val) : impl_type::node_base(std::move(val)) {}
    };

    using node_alloc = typename allocator_type::template rebind<node>::other;
    using list_type = boost::intrusive::list<
        node,
        boost::intrusive::member_hook<node, list_hook, &node::list_> >;
    using hash_type = boost::intrusive::unordered_set<
        node,
        boost::intrusive::member_hook<node, hash_hook, &node::hash_>,
        boost::intrusive::constant_time_size<false> >;
    using bucket_type = typename hash_type::bucket_type;
    using bucket_ptr = typename hash_type::bucket_ptr;
    using bucket_alloc = typename allocator_type::template rebind<bucket_type>::other;
    using bucket_traits = typename hash_type::bucket_traits;

    struct bucket_deleter;
    static std::unique_ptr<bucket_type, bucket_deleter> make_bucket(bucket_alloc alloc, size_type size);
    template <typename... Args>
    node & new_node(Args&&... args);
    void del_node(node const & val);
    void rehash(size_type size);
    void auto_rehash();

private:
    std::unique_ptr<bucket_type, bucket_deleter> bucket_;
    list_type list_;
    hash_type hash_;
};

} } }

#include <acqua/container/detail/lru_cache_facade.ipp>
