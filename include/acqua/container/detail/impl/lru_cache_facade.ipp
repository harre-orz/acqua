/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/container/detail/lru_cache_facade.hpp>

namespace acqua { namespace container { namespace detail {


template <typename Impl>
struct lru_cache_facade<Impl>::bucket_deleter
{
    bucket_deleter(bucket_alloc alloc, size_type size)
        : alloc_(alloc), size_(size)
    {
    }

    void operator()(bucket_type * ptr)
    {
        using alloc_traits = std::allocator_traits<bucket_alloc>;
        for(size_type i = 0; i < size_; ++i)
            alloc_traits::destroy(alloc_, ptr + i);
        alloc_traits::deallocate(alloc_, ptr, size_);
    }

    bucket_alloc alloc_;
    size_type size_;
};


template <typename Impl>
inline auto lru_cache_facade<Impl>::make_bucket(bucket_alloc alloc, size_type size) -> std::unique_ptr<bucket_type, bucket_deleter>
{
    using alloc_traits = std::allocator_traits<bucket_alloc>;

    bucket_ptr ptr = alloc_traits::allocate(alloc, size);
    for(size_type i = 0; i < size; ++i)
        alloc_traits::construct(alloc, ptr + i);
    return std::unique_ptr<bucket_type, bucket_deleter>(ptr, bucket_deleter(alloc, size));
}


template <typename Impl> template <typename... Args>
inline auto lru_cache_facade<Impl>::new_node(Args&&... args) -> node &
{
    using alloc_traits = std::allocator_traits<node_alloc>;

    node_alloc alloc(*this);
    auto * ptr = alloc_traits::allocate(alloc, 1);
    alloc_traits::construct(alloc, ptr, args...);
    return *ptr;
}


template <typename Impl>
inline void lru_cache_facade<Impl>::del_node(node const & val)
{
    using alloc_traits = std::allocator_traits<node_alloc>;

    node_alloc alloc(*this);
    auto * ptr = const_cast<node *>(&val);
    alloc_traits::destroy(alloc, ptr);
    alloc_traits::deallocate(alloc, ptr, 1);
}


template <typename Impl>
inline void lru_cache_facade<Impl>::rehash(size_type size)
{
    auto bucket = make_bucket(*this, size);
    hash_.rehash(bucket_traits(bucket.get(), size));
    bucket_ = std::move(bucket);
}


template <typename Impl>
inline void lru_cache_facade<Impl>::auto_rehash()
{
    if (size() > hash_.bucket_count()) {
        rehash(hash_.bucket_count() * 2);
    } else if (size() > min_bucket_size && size() < hash_.bucket_count() / 2) {
        rehash(hash_.bucket_count() / 2);
    }
}

template <typename Impl>
struct lru_cache_facade<Impl>::iterator
    :  boost::iterator_adaptor<iterator, typename list_type::iterator, value_type>
{
    iterator() = default;


    iterator(typename list_type::iterator it)
        : boost::iterator_adaptor<iterator, typename list_type::iterator, value_type>(it)
    {
    }

    value_type & operator*() const
    {
        return this->base()->value_;
    }

    value_type * operator->() const
    {
        return &this->base()->value_;
    }
};


template <typename Impl>
struct lru_cache_facade<Impl>::const_iterator
    : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>
{
    const_iterator() = default;

    const_iterator(typename list_type::const_iterator it)
        : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>(it)
    {
    }

    const_iterator(iterator it)
        : boost::iterator_adaptor<const_iterator, typename list_type::const_iterator, value_type>(it.base())
    {
    }

    value_type const & operator*() const
    {
        return this->base()->value_;
    }

    value_type const * operator->() const
    {
        return &this->base()->value_;
    }
};


template <typename Impl>
struct lru_cache_facade<Impl>::reverse_iterator
    : boost::iterator_adaptor<reverse_iterator, typename list_type::reverse_iterator, value_type>
{
    reverse_iterator() = default;

    reverse_iterator(typename list_type::reverse_iterator it)
        : boost::iterator_adaptor<reverse_iterator, typename list_type::reverse_iterator, value_type>(it)
    {
    }

    value_type & operator*() const
    {
        return this->base()->value_;
    }

    value_type * operator->() const
    {
        return &this->base()->value_;
    }
};


template <typename Impl>
struct lru_cache_facade<Impl>::const_reverse_iterator
    : public boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>
{
    const_reverse_iterator() = default;

    const_reverse_iterator(typename list_type::const_reverse_iterator it)
        : boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>(it)
    {
    }

    const_reverse_iterator(reverse_iterator it)
        : boost::iterator_adaptor<const_reverse_iterator, typename list_type::const_reverse_iterator, value_type>(it.base())
    {
    }

    value_type const & operator*() const
    {
        return this->base()->value_;
    }

    value_type const * operator->() const
    {
        return this->base()->value_;
    }
};


template <typename Impl>
inline lru_cache_facade<Impl>::lru_cache_facade(allocator_type alloc, size_type size)
    : allocator_type(alloc)
    , bucket_(make_bucket(alloc, size))
    , hash_(bucket_traits(bucket_.get(), size))
{
}


template <typename Impl>
inline lru_cache_facade<Impl>::lru_cache_facade(lru_cache_facade const & rhs)
    : allocator_type(rhs.get_allocator())
    , bucket_(make_bucket(*this, rhs.hash_.bucket_count()))
    , hash_(bucket_traits(bucket_.get(), rhs.hash_.bucket_count()))
{
    for(auto const & e : rhs)
        push(e);
}


template <typename Impl>
inline lru_cache_facade<Impl>::lru_cache_facade(lru_cache_facade &&) = default;


template <typename Impl>
inline lru_cache_facade<Impl>::~lru_cache_facade()
{
    clear();
}


template <typename Impl>
inline lru_cache_facade<Impl> & lru_cache_facade<Impl>::operator=(lru_cache_facade<Impl> &&) = default;


template <typename Impl>
inline lru_cache_facade<Impl> & lru_cache_facade<Impl>::operator=(lru_cache_facade const & rhs)
{
    if (this != &rhs) {
        clear();
        for(auto const & e : rhs)
            push(e);
    }
    return *this;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::get_allocator() const -> allocator_type
{
    return *this;
}


template <typename Impl>
inline bool lru_cache_facade<Impl>::empty() const
{
    return list_.empty();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::size() const -> size_type
{
    return list_.size();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::begin() -> iterator
{
    return list_.begin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::begin() const -> const_iterator
{
    return list_.begin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::end() -> iterator
{
    return list_.end();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::end() const -> const_iterator
{
    return list_.end();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::rbegin() -> reverse_iterator
{
    return list_.rbegin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::rbegin() const -> const_reverse_iterator
{
    return list_.rbegin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::rend() -> reverse_iterator
{
    return list_.rend();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::rend() const -> const_reverse_iterator
{
    return list_.rend();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::cbegin() const -> const_iterator
{
    return list_.begin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::cend() const -> const_iterator
{
    return list_.end();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::crbegin() const -> const_reverse_iterator
{
    return list_.rbegin();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::crend() const -> const_reverse_iterator
{
    return list_.rend();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::front() -> value_type &
{
    return list_.front().value_;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::front() const -> value_type const &
{
    return list_.front().value_;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::back() -> value_type &
{
    return list_.back().value_;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::back() const -> value_type const &
{
    return list_.back().value_;
}


template <typename Impl>
inline void lru_cache_facade<Impl>::clear()
{
    erase(begin(), end());
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::find(key_type const & key) -> iterator
{
    auto it = hash_.find(key, hasher(), equal_to());
    return (it != hash_.end()) ? list_.iterator_to(*it) : list_.end();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::find(key_type const & key) const -> const_iterator
{
    auto it = hash_.find(key, hasher(), equal_to());
    return (it != hash_.end()) ? list_.iterator_to(*it) : list_.end();
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::erase(const_iterator it) -> iterator
{
    auto pos = end();
    if (it != pos) {
        node const & val = *it.base();
        pos = list_.erase(list_.iterator_to(val));
        hash_.erase(hash_.iterator_to(val));
        del_node(val);
    }
    return pos;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::erase(const_iterator beg, const_iterator end) -> iterator
{
    auto pos = this->end();
    while(beg != end) {
        node const & val = *beg.base();
        beg = pos = list_.erase(list_.iterator_to(val));
        hash_.erase(hash_.iterator_to(val));
        del_node(val);
    }
    return pos;
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::erase(key_type const & key) -> iterator
{
    return base_type::erase(base_type::find(key));
}


template <typename Impl> template <typename U>
inline bool lru_cache_facade<Impl>::push(U && val)
{
    auto it = hash_.find(val, hasher(), equal_to());
    if (it != hash_.end()) {
        // update
        auto & node = *it;
        list_.erase(list_.iterator_to(node));
        node.replace(std::forward<U>(val));
        list_.push_front(node);
        return false;
    } else if (!list_.empty() && impl_type::is_limits(*this)) {
        // replace
        auto & node = list_.back();
        list_.erase(list_.iterator_to(node));
        hash_.erase(hash_.iterator_to(node));
        node.replace(std::forward<U>(val));
        list_.push_front(node);
        hash_.insert(node);
    } else {
        // new
        auto & node = new_node(std::forward<U>(val));
        hash_.insert(node);
        list_.push_front(node);
        auto_rehash();
    }
    return true;
}


template <typename Impl>
inline void lru_cache_facade<Impl>::pop()
{
    if (!list_.empty()) {
        auto & node = list_.back();
        list_.erase(list_.iterator_to(node));
        hash_.erase(hash_.iterator_to(node));
        del_node(*this, node);
    }
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::insert(value_type const & val) -> std::pair<iterator, bool>
{
    bool res = push(val);
    return std::make_pair(begin(), res);
}


template <typename Impl>
inline auto lru_cache_facade<Impl>::insert(const_iterator, value_type const & val) -> iterator
{
    push(val);
    return begin();
}


template <typename Impl> template <typename... Args>
inline auto lru_cache_facade<Impl>::emplace(Args&&... args) -> std::pair<iterator, bool>
{
    bool res = base_type::push(value_type(args...));
    return std::make_pair(begin(), res);
}


template <typename Impl> template <typename... Args>
inline auto lru_cache_facade<Impl>::emplace_hint(const_iterator, Args&&... args) -> iterator
{
    base_type::push(value_type(args...));
    return begin();
}


template <typename Impl>
inline void lru_cache_facade<Impl>::shrink_to_fit()
{
    while(!empty() && impl_type::is_limits(*this))
        erase(--end());
    auto_rehash();
}

} } }
