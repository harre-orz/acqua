/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/email/email.hpp>

namespace acqua { namespace email {

template <typename String>
inline basic_email<String>::basic_email()
    : impl_(new message_type())
{
}

template <typename String>
inline auto basic_email<String>::begin() -> iterator
{
    return impl_ ? iterator(*impl_) : iterator();
}


template <typename String>
inline auto basic_email<String>::begin() const -> const_iterator
{
    return impl_ ? const_iterator(*impl_) : const_iterator();
}


template <typename String>
inline auto basic_email<String>::end() -> iterator
{
    return iterator();
}


template <typename String>
inline auto basic_email<String>::end() const -> const_iterator
{
    return const_iterator();
}


// template <typename String>
// inline bool basic_email<String>::add_attachment(istream_type & is, value_type const & contenttype)
// {
//     std::unique_ptr<message_type> main;
//     // impl_ がサブパートを持たない場合、impl_ をサブパートの位置に移動させ、マルチパートに書き換える
//     if (impl_->has_subpart()) {
//     }
// }

} }
