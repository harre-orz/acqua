/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace container { namespace detail {

template <typename T, typename It, It (*begin)(T &), It (*end)(T &), typename Stack>
struct preordered_recursive_iterator_impl
{
    using value_type = T;
    using stack_type = Stack;
    using size_type = typename stack_type::size_type;

    preordered_recursive_iterator_impl(value_type * root);

    void incr();

    value_type * root_;
    stack_type depth_;
};

} } }

#include <acqua/container/detail/impl/preordered_recursive_iterator_impl.ipp>
