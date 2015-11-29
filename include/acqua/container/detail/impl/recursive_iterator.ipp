#pragma once

#include <acqua/container/detail/recursive_iterator.hpp>

namespace acqua { namespace container { namespace detail {

template <typename Impl>
inline recursive_iterator<Impl>::recursive_iterator()
    : Impl(nullptr)
{
}


template <typename Impl>
inline recursive_iterator<Impl>::recursive_iterator(value_type & root)
    : Impl(&root)
{
}


template <typename Impl>
inline auto recursive_iterator<Impl>::operator*() const -> reference
{
    return depth_.empty() ? *root_ : *depth_.front();
}


template <typename Impl>
inline auto recursive_iterator<Impl>::operator->() const -> pointer
{
    return &operator*();
}


template <typename Impl>
inline recursive_iterator<Impl> & recursive_iterator<Impl>::operator++()
{
    incr();
    return *this;
}


template <typename Impl>
inline std::size_t recursive_iterator<Impl>::depth() const
{
    return Impl::depth_.size();
}


template <typename Impl>
inline bool operator==(recursive_iterator<Impl> const & lhs, recursive_iterator<Impl> const & rhs)
{
    return (lhs.depth_.empty()) ? (rhs.depth_.empty() ? lhs.root_ == rhs.root_ : false)
        :  (rhs.depth_.empty()) ? false
        :  lhs.depth_.front() == rhs.depth_.front();
}


template <typename Impl>
inline bool operator!=(recursive_iterator<Impl> const & lhs, recursive_iterator<Impl> const & rhs)
{
    return !(lhs == rhs);
}

} } }
