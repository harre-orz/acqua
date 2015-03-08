/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/deadline_timer.hpp>

namespace acqua { namespace asio {

//! boost::asio で定義されるタイマーの特性クラス.
template <typename AsioTimer>
class timer_traits
{
public:
    using timer_type = AsioTimer;
    using time_point_type = typename timer_type::time_point;
    using duration_type = typename timer_type::duration;

    static time_point_type now()
    {
        return timer_type::clock_type::now();
    }
};

//! boost::asio::deadline_timer の特殊化.
template <>
class timer_traits<boost::asio::deadline_timer>
{
public:
    using timer_type = boost::asio::deadline_timer;
    using time_point_type = typename timer_type::time_type;
    using duration_type = typename timer_type::duration_type;

    static time_point_type now()
    {
        return timer_type::traits_type::now();
    }
};

} }
