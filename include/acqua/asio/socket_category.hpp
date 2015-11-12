/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace asio {

//! どのタイプか不明なことを示すタグ
class unspecified_tag {};

//! IPv4用であることを示すタグ
class internet_v4_tag {};

//! IPv6用であることを示すタグ
class internet_v6_tag {};

//! UNNIX Domainソケット用であることを示すタグ
class unix_local_tag {};

} }
