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

/*!
 * コンテナ T を unique_ptr<T> で管理されたイテレータでラップしたクラス.
 *
 * コンテナの終端 end() を内部で保持するため、イテレータの終端が不要になる
 */
template <
    typename T,
    typename Iter = typename T::iterator >
class unique_container_iterator : public acqua::container::detail::pointer_container_iterator<
    typename T::value_type,
    Iter,
    std::unique_ptr<T>
    >
{
    using base_type = typename unique_container_iterator::base_type;

public:
    using base_type::base_type;
};


/*!
 * 読み取り専用のコンテナ T を unique_ptr<T const> で管理されたイテレータでラップしたクラス.
 *
 * コンテナの終端 end() を内部で保持するため、イテレータの終端が不要になる
 */
template <typename T, typename Iter>
class unique_container_iterator<T const, Iter> : public acqua::container::detail::pointer_container_iterator<
    typename T::value_type const,
    Iter,
    std::unique_ptr<T>
    >
{
    using base_type = typename unique_container_iterator::base_type;

public:
    using base_type::base_type;
};

} }
