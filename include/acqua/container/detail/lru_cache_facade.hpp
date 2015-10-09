/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/config.hpp>
#include <iostream>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/unordered_set.hpp>
#include <boost/iterator_adaptors.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

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

public:
    ACQUA_DECL lru_cache_facade(allocator_type alloc, size_type size);

    ACQUA_DECL lru_cache_facade(lru_cache_facade const & rhs);

    ACQUA_DECL lru_cache_facade(lru_cache_facade && rhs) = default;

    ACQUA_DECL ~lru_cache_facade();

    ACQUA_DECL lru_cache_facade & operator=(lru_cache_facade const & rhs);

    ACQUA_DECL lru_cache_facade & operator=(lru_cache_facade && rhs) = default;

    ACQUA_DECL allocator_type get_allocator() const;

    ACQUA_DECL bool empty() const;

    ACQUA_DECL size_type size() const;

    ACQUA_DECL iterator begin();

    ACQUA_DECL const_iterator begin() const;

    ACQUA_DECL iterator end();

    ACQUA_DECL const_iterator end() const;

    ACQUA_DECL reverse_iterator rbegin();

    ACQUA_DECL const_reverse_iterator rbegin() const;

    ACQUA_DECL reverse_iterator rend();

    ACQUA_DECL const_reverse_iterator rend() const;

    ACQUA_DECL value_type & front();

    ACQUA_DECL value_type const & front() const;

    ACQUA_DECL value_type & back();

    ACQUA_DECL value_type const & back() const;

    ACQUA_DECL void clear();

    ACQUA_DECL iterator find(key_type const & key);

    ACQUA_DECL const_iterator find(key_type const & key) const;

    ACQUA_DECL iterator erase(const_iterator it);

    ACQUA_DECL iterator erase(const_iterator beg, const_iterator end);

    ACQUA_DECL bool push(value_type const & val);

    ACQUA_DECL bool push(value_type && val);

    ACQUA_DECL void pop();

    ACQUA_DECL size_type node_element_size() const;

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
        ACQUA_DECL explicit node(value_type const & val, lru_cache_facade const & facade) : impl_type::node_base(val, facade) {}
        ACQUA_DECL explicit node(value_type && val, lru_cache_facade const & facade) : impl_type::node_base(val, facade) {}
    };

    using node_alloc = typename allocator_type::template rebind<node>::other;
    using list_type = boost::intrusive::list<node, boost::intrusive::member_hook<node, list_hook, &node::list_> >;
    using hash_type = boost::intrusive::unordered_set<node, boost::intrusive::member_hook<node, hash_hook, &node::hash_> >;
    using bucket_type = typename hash_type::bucket_type;
    using bucket_ptr = typename hash_type::bucket_ptr;
    using bucket_alloc = typename allocator_type::template rebind<bucket_type>::other;
    using bucket_traits = typename hash_type::bucket_traits;

    struct bucket_deleter;
    ACQUA_DECL static std::unique_ptr<bucket_type, bucket_deleter> make_bucket(bucket_alloc alloc, size_type size);
    ACQUA_DECL node & new_node(value_type const & val);
    ACQUA_DECL node & new_node(value_type && val);
    ACQUA_DECL void   del_node(node const & val);

private:
    std::unique_ptr<bucket_type, bucket_deleter> bucket_;
    list_type list_;
    hash_type hash_;
};

} } }

#include <acqua/container/detail/impl/lru_cache_facade.ipp>
