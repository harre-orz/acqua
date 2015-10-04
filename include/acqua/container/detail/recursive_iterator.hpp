/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>

namespace acqua { namespace container { namespace detail {

template <typename Impl>
class recursive_iterator
    : public std::iterator<std::forward_iterator_tag, typename Impl::value_type>
    , private Impl
{
    friend Impl;
    using Impl::root_;
    using Impl::depth_;
    using Impl::incr;

protected:
    using base_type = recursive_iterator;

public:
    using value_type = typename Impl::value_type;
    using reference = value_type &;
    using pointer = value_type *;

public:
    recursive_iterator();

    recursive_iterator(recursive_iterator const & rhs) = default;

    recursive_iterator(recursive_iterator && rhs) = default;

    explicit recursive_iterator(value_type & root);

    recursive_iterator & operator=(recursive_iterator const & rhs) = default;

    recursive_iterator & operator=(recursive_iterator && rhs) = default;

    reference operator*() const;

    pointer operator->() const;

    recursive_iterator & operator++();

    std::size_t depth() const;

    template <typename Impl_>
    friend bool operator==(recursive_iterator<Impl_> const lhs, recursive_iterator<Impl_> const & rhs);

    template <typename Impl_>
    friend bool operator!=(recursive_iterator<Impl_> const lhs, recursive_iterator<Impl_> const & rhs);
};

} } }

#include <acqua/container/detail/impl/recursive_iterator.ipp>
