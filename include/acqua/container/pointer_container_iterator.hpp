/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <type_traits>

namespace acqua { namespace container {

/*!
  コンテナ T のポインタをイテレータに内包したデータクラス.

  イテレータの end を取得できない場合でも it != pointer_container_iterator() で判定できる
 */
template <typename T, typename ManagedPtr>
class pointer_container_iterator
    : public std::iterator<std::forward_iterator_tag, typename T::value_type>
{
public:
    using managed_ptr = ManagedPtr;
    using value_type = typename T::value_type;
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;
    using specified_iterator = typename std::conditional<std::is_const<T>::value, const_iterator, iterator>::type;
    using specified_value_type = typename std::conditional<std::is_const<T>::value, value_type const, value_type>::type;

    pointer_container_iterator() noexcept = default;
    pointer_container_iterator(pointer_container_iterator const &) noexcept = default;
    pointer_container_iterator(pointer_container_iterator &&) noexcept = default;

    pointer_container_iterator(managed_ptr const & ptr) noexcept
        : ptr_(ptr), it_(ptr_->begin()) {}

    template <typename It>
    pointer_container_iterator(It it, managed_ptr const & ptr) noexcept
        : ptr_(ptr), it_(it) {}

    pointer_container_iterator(T * ptr) noexcept
        : ptr_(ptr), it_(ptr_->begin()) {}

    template <typename It>
    pointer_container_iterator(It it, T * ptr) noexcept
        : ptr_(ptr), it_(it) {}

    specified_value_type & operator*() noexcept
    {
        return *it_;
    }

    specified_value_type & operator*() const noexcept
    {
        return *it_;
    }

    specified_value_type * operator->() noexcept
    {
        return &*it_;
    }

    specified_value_type * operator->() const noexcept
    {
        return &*it_;
    }

    bool operator==(pointer_container_iterator const & rhs) const noexcept
    {
        if (ptr_ && rhs.ptr_) {
            if (ptr_ == rhs.ptr_ && it_ != rhs.it_) return false;
        } else if (ptr_) {
            if (it_ != ptr_->end()) return false;
        } else if (rhs.ptr_) {
            if (rhs.it_ != rhs.ptr_->end()) return false;
        }

        return true;
    }

    bool operator!=(pointer_container_iterator const & rhs) const noexcept
    {
        return !operator==(rhs);
    }

    pointer_container_iterator & operator++() noexcept
    {
        ++it_;
        return *this;
    }

    pointer_container_iterator operator++(int) noexcept
    {
        pointer_container_iterator it(*this);
        operator++();
        return it;
    }

    T * get() noexcept
    {
        return ptr_.get();
    }

    T * get() const noexcept
    {
        return ptr_.get();
    }

private:
    managed_ptr ptr_;
    specified_iterator it_;
};

} }


#include <memory>
#include <iterator>

namespace acqua { namespace container {

template <typename T>
using shared_container_iterator = pointer_container_iterator<T, std::shared_ptr<T> >;

template <typename T>
using unique_container_iterator = pointer_container_iterator<T, std::unique_ptr<T> >;

} }
