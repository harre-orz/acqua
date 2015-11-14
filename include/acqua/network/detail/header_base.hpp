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
protected:
    ~header_base() = default;

public:
    //! ヘッダーの有効サイズを返す.
    constexpr std::size_t header_size() const noexcept { return sizeof(Derived); }

    //! データ長の終端をヘッダー情報より算出した終了位置にする
    template <typename It>
    constexpr void shrink_into_end(It &) const noexcept {}
};

} } }
