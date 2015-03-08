/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/utility/move_on_copy_wrapper.hpp>

namespace acqua {

/*!
  move を copy で行うラッパークラスを作成する.

  std::bind の引数などで、move が使えないときに使用することを想定している。乱用は厳禁
 */
template <typename T>
inline utility::move_on_copy_wrapper< typename std::remove_reference<T>::type > mref(T t)
{
    return utility::move_on_copy_wrapper< typename std::remove_reference<T>::type >( std::move(t) );
}

}
