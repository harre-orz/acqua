/*!
  The acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace network { namespace detail {

/*!
  T1 から T2 に変換できるかを表す. 変換出来る場合は、特種化したものを定義する
 */
template <typename T1, typename T2>
class is_match_condition;

} } }
