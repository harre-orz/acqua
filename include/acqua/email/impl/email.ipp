#pragma once

#include <acqua/email/email.hpp>

namespace acqua { namespace email {

template <typename String>
basic_email<String>::basic_email()
    : impl_(new message_type())
{
}

template <typename String>
bool basic_email<String>::add_attachment(istream_type & is, value_type const & contenttype)
{
    std::unique_ptr<message_type> main;
    // impl_ がサブパートを持たない場合、impl_ をサブパートの位置に移動させ、マルチパートに書き換える
    if (impl_->has_subpart()) {
    }

}

} }
