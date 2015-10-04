#pragma once

namespace acqua { namespace container { namespace detail {

template <typename T, typename It, It (*begin)(T &), It (*end)(T &), typename Stack>
struct preordered_recursive_iterator_impl
{
    using value_type = T;
    using stack_type = Stack;

    ACQUA_DECL preordered_recursive_iterator_impl(value_type * root);

    ACQUA_DECL void incr();

    value_type * root_;
    stack_type depth_;
};

} } }

#include <acqua/container/detail/impl/preordered_recursive_iterator_impl.ipp>
