#pragma once

#include <acqua/email/basic_email.hpp>

namespace acqua { namespace email {

using email = basic_email<std::string>;
using wemail = basic_email<std::wstring>;

} }
