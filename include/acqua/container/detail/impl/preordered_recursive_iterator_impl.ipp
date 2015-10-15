/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/container/detail/preordered_recursive_iterator_impl.hpp>

namespace acqua { namespace container  { namespace detail {

template <typename T, typename It, It (*begin)(T &), It (*end)(T &), typename Stack>
inline preordered_recursive_iterator_impl<T, It, begin, end, Stack>::preordered_recursive_iterator_impl(value_type * root)
    : root_(root)
{
}


template <typename T, typename It, It (*begin)(T &), It (*end)(T &), typename Stack>
inline void preordered_recursive_iterator_impl<T, It, begin, end, Stack>::incr()
{
    auto it = depth_.empty()
        ? (*begin)(*root_)
        : (*begin)(*depth_.front());
    if (depth_.empty()) {
        if (it == (*end)(*root_)) {
            root_ = nullptr;
        } else {
            depth_.emplace_back(it);
        }
    } else {
        if (it == (*end)(*depth_.front())) {
            it = ++depth_.front();
            depth_.pop_back();
            incr();
        } else {
            depth_.emplace_back(it);
        }
    }
}

} } }
