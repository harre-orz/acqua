/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/email/basic_email.hpp>

namespace acqua { namespace email {

using email = basic_email<std::string>;
using wemail = basic_email<std::wstring>;

} }
