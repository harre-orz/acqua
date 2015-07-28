#pragma once

#include <iostream>
#include <acqua/email/basic_message.hpp>
#include <acqua/email/basic_address.hpp>
#include <acqua/email/email_traits.hpp>

namespace acqua { namespace email {

template <
    typename String,
    typename Traits = email_traits<String>
    >
class basic_email
    : public basic_message<String>
{
    using base_type = basic_message<String>;

};

} }
