#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <iostream>
#include <string>

namespace acqua { namespace email {

template <typename CharT>
void encode_mimeheader(std::ostream & os, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, std::string const & charset = "UTF-8");

template <typename CharT, typename Params>
void encode_mimeheader(std::ostream & os, std::basic_string<CharT> const & key, std::basic_string<CharT> const & val, Params const & params, std::string const & charset = "UTF-8");

} }

#include <acqua/email/encode_mimeheader.ipp>
