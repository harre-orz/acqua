/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <acqua/asio/basic_notify.hpp>
#include <acqua/asio/detail/inotify_service.hpp>

namespace acqua { namespace asio {

typedef basic_notify<detail::inotify_service> inotify;

} }
