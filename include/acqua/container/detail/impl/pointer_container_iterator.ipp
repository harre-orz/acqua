#pragma once

#include <acqua/container/detail/pointer_container_iterator.hpp>

namespace acqua { namespace container { namespace detail {

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(ManagedPtr const & ptr)
    : ptr_(ptr)
    , it_(ptr_->begin())
{
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(ManagedPtr const & ptr, Iter it)
    : ptr_(ptr)
    , it_(it)
{
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(managed_ptr && ptr)
    : ptr_(std::move(ptr))
    , it_(ptr_->begin())
{
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(managed_ptr && ptr, Iter it)
    : ptr_(std::move(ptr))
    , it_(it)
{
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(element_type * ptr)
    : ptr_(ptr)
{
    if (ptr_)
        it_ = ptr_->begin();
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr>::pointer_container_iterator(element_type * ptr, Iter it)
    : ptr_(ptr)
{
    if (ptr_)
        it_ = it;
}

template <typename T, typename Iter, typename ManagedPtr>
inline auto pointer_container_iterator<T, Iter, ManagedPtr>::operator*() const -> reference
{
    return *it_;
}

template <typename T, typename Iter, typename ManagedPtr>
auto pointer_container_iterator<T, Iter, ManagedPtr>::operator->() const -> pointer
{
    return &*it_;
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr> & pointer_container_iterator<T, Iter, ManagedPtr>::operator++()
{
    ++it_;
    return *this;
}

template <typename T, typename Iter, typename ManagedPtr>
inline pointer_container_iterator<T, Iter, ManagedPtr> pointer_container_iterator<T, Iter, ManagedPtr>::operator++(int)
{
    pointer_container_iterator it(*this);
    ++it_;
    return it;
}

template <typename T, typename Iter, typename ManagedPtr>
inline typename pointer_container_iterator<T, Iter, ManagedPtr>::element_type * pointer_container_iterator<T, Iter, ManagedPtr>::get() const
{
    return ptr_.get();
}

template <typename T, typename Iter, typename ManagedPtr>
inline bool operator==(pointer_container_iterator<T, Iter, ManagedPtr> const & lhs, pointer_container_iterator<T, Iter, ManagedPtr> const & rhs)
{
    if (lhs.ptr_ && rhs.ptr_) {
        if (lhs.ptr_ == rhs.ptr_ && lhs.it_ != rhs.it_) return false;
    } else if (lhs.ptr_) {
        if (lhs.it_ != lhs.ptr_->end()) return false;
    } else if (rhs.ptr_) {
        if (rhs.it_ != rhs.ptr_->end()) return false;
    }

    return true;
}

template <typename T, typename Iter, typename ManagedPtr>
inline bool operator!=(pointer_container_iterator<T, Iter, ManagedPtr> const & lhs, pointer_container_iterator<T, Iter, ManagedPtr> const & rhs)
{
    return !(lhs == rhs);
}

} } }
