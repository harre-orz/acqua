/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <boost/algorithm/string/trim.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/assign.hpp>
#include <boost/assign/list_inserter.hpp>
#include <acqua/email/email_fwd.hpp>

namespace acqua { namespace email {

/*!
  メールアドレスクラス.
 */
template <typename String>
class basic_address
{
public:
    using value_type = String;

    basic_address() = default;

    basic_address(value_type const & addrspec)
        : namespec(), addrspec(addrspec) {}

    basic_address(value_type const & addrspec, value_type const & namespec)
        : namespec(namespec), addrspec(addrspec) {}

public:
    value_type namespec;
    value_type addrspec;
};

} }
