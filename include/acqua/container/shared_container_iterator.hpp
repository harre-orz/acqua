/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <memory>
#include <type_traits>
#include <acqua/container/detail/pointer_container_iterator.hpp>

namespace acqua { namespace container {

template <typename T, typename Iter = typename T::iterator>
class unique_container_iterator : public acqua::container::detail::pointer_container_iterator<
    typename T::value_type,
    Iter,
    std::shared_ptr<T>
    >
{
    using base_type = typename shared_container_iterator;

public:
    using base_type::base_type;
};

template <typename T, typename Iter>
class unique_container_iterator<T const, Iter> : public acqua::container::detail::pointer_container_iterator<
    typename T::value_type const,
    Iter,
    std::shared_ptr<T>
    >
{
    using base_type = typename shared_container_iterator;

public:
    using base_type::base_type;
};

} }
