/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <deque>
#include <acqua/container/detail/preordered_recursive_iterator_impl.hpp>

namespace acqua { namespace container {

template <typename Impl>
class basic_recursive_iterator
    : public std::iterator<std::forward_iterator_tag, typename Impl::value_type>
    , private Impl
{
    using Impl::root_;
    using Impl::depth_;
    using Impl::incr;

public:
    using value_type = typename Impl::value_type;
    using size_type = typename Impl::size_type;
    using reference = value_type &;
    using pointer = value_type *;

public:
    basic_recursive_iterator()
        : Impl(nullptr) {}

    basic_recursive_iterator(basic_recursive_iterator const &) = default;

    basic_recursive_iterator(basic_recursive_iterator &&) = default;

    explicit basic_recursive_iterator(value_type & root)
        : Impl(&root) {}

    basic_recursive_iterator & operator=(basic_recursive_iterator const &) = default;

    basic_recursive_iterator & operator=(basic_recursive_iterator &&) = default;

    reference operator*() const
    {
        return depth_.empty() ? *root_ : *depth_.front();
    }

    pointer operator->() const
    {
        return &operator*();
    }

    basic_recursive_iterator & operator++()
    {
        incr();
        return *this;
    }

    size_type depth() const
    {
        return depth_.size();
    }

    template <typename Impl_>
    friend bool operator==(basic_recursive_iterator<Impl_> const & lhs, basic_recursive_iterator<Impl_> const & rhs)
    {
        return (lhs.depth_.empty()) ? (rhs.depth_.empty() ? lhs.root_ == rhs.root_ : false)
            :  (rhs.depth_.empty()) ? false
            :  lhs.depth_.front() == rhs.depth_.front();
    }

    template <typename Impl_>
    friend bool operator!=(basic_recursive_iterator<Impl_> const & lhs, basic_recursive_iterator<Impl_> const & rhs)
    {
        return !(lhs == rhs);
    }
};

template <
    typename T,
    typename It = typename T::iterator,
    It (*begin)(T &) = &std::begin<T>,
    It (*end)(T &) = &std::end<T>,
    typename Stack = std::deque<It>
    >
using preordered_recursive_iterator =
    basic_recursive_iterator< detail::preordered_recursive_iterator_impl<T, It, begin, end, Stack> >;

} }
