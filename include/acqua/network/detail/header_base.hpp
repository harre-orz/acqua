/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace network { namespace detail {

template <typename Derived>
class header_base
{
public:
    //! ヘッダーの有効サイズを返す.
    constexpr std::size_t size() const { return sizeof(Derived); }

    //! end を縮小させる.
    template <typename It>
    void shrink(It &) const {}
};

} } }
