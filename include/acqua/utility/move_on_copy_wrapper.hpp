/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <utility>
#include <type_traits>

namespace acqua { namespace utility {

/*!
  move セマンティクスを copy で実現するラッパークラス.

  このクラスを扱うときは、所有権がどこにあるかを十分に留意して実装すること
 */
template <typename T>
class move_on_copy_wrapper
{
public:
    using value_type = T;

    move_on_copy_wrapper() = delete;

    move_on_copy_wrapper(move_on_copy_wrapper const & rhs)
        : value_(std::move(rhs.value_)) {}

    move_on_copy_wrapper(move_on_copy_wrapper && rhs)
        : value_(std::move(rhs.value_)) {}

    move_on_copy_wrapper(T && t)
        : value_(std::move(t))
    {
        static_assert(std::is_reference<T>::value == false, "T can't be defined as (left/right) reference.");
    }

    move_on_copy_wrapper & operator=(move_on_copy_wrapper const & rhs)
    {
        if (this != &rhs)
            value_ = std::move(rhs.value_);
        return *this;
    }

    move_on_copy_wrapper & operator=(move_on_copy_wrapper && rhs)
    {
        if (this != &rhs)
            value_ = std::move(rhs.value_);
        return *this;
    }

    operator T()
    {
        return std::move(value_);
    }

    operator T() const
    {
        return std::move(value_);
    }

    T & get()
    {
        return value_;
    }

    T const & get() const
    {
        return value_;
    }

private:
    mutable value_type value_;
};

} }
