/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace network { namespace detail {

template <typename T>
class pseudo_header;

template <typename Derived>
class header_base
{
protected:
    ~header_base() = default;

    //! ヘッダーの有効サイズを返す. IPヘッダーは可変長になる
    constexpr std::size_t size() const noexcept
    {
        return sizeof(Derived);
    }

    //! end を縮小させる. IPヘッダーに含まれるデータサイズに応じてendを小さくする
    template <typename It>
    void shrink(It &) const noexcept
    {
    }
};

} } }
