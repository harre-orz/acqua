/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <deque>
#include <acqua/container/detail/recursive_iterator.hpp>
#include <acqua/container/detail/preordered_recursive_iterator_impl.hpp>

namespace acqua { namespace container {

template <
    typename T,
    typename It = typename T::iterator,
    It (*begin)(T &) = &std::begin<T>,
    It (*end)(T &) = &std::end<T>,
    typename Stack = std::deque<It>
    >
class preordered_recursive_iterator
    : public detail::recursive_iterator< detail::preordered_recursive_iterator_impl<T, It, begin, end, Stack> >
{
    using base_type = typename preordered_recursive_iterator::base_type;

public:
    using base_type::base_type;
};

} }
