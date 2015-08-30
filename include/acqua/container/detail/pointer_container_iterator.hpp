/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>
#include <iostream>
namespace acqua { namespace container { namespace detail {

/*!
  コンテナ T のポインタをイテレータに内包したデータクラス.

  イテレータの end を取得できない場合でも it != pointer_container_iterator() で判定できる
 */
template <typename T, typename Iter, typename ManagedPtr>
class pointer_container_iterator
    : public std::iterator<std::forward_iterator_tag, typename Iter::value_type>
{
protected:
    using base_type = pointer_container_iterator;

public:
    using iterator = Iter;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    using managed_ptr = ManagedPtr;
    using element_type = typename managed_ptr::element_type;

public:
    pointer_container_iterator() = default;
    pointer_container_iterator(pointer_container_iterator const & rhs) = default;
    pointer_container_iterator(pointer_container_iterator && rhs) = default;

    pointer_container_iterator(iterator it, managed_ptr const & ptr)
        : ptr_(ptr)
        , it_(it)
    {
    }

    pointer_container_iterator(iterator it, managed_ptr && ptr)
        : ptr_(std::move(ptr))
        , it_(it)
    {
    }

    template <typename U, typename std::enable_if<std::is_convertible<U, element_type>::value>::type * = nullptr>
    pointer_container_iterator(U * ptr)
        : ptr_(ptr)
        , it_(ptr_->begin())
    {
    }

    friend bool operator==(pointer_container_iterator const & lhs, pointer_container_iterator const & rhs)
    {
        if (lhs.ptr_ && rhs.ptr_) {
            if (lhs.ptr_ == rhs.ptr_ && lhs.it_ != rhs.it_) return false;
        } else if (lhs.ptr_) {
            if (lhs.it_ != lhs.ptr_->end()) return false;
        } else if (rhs.ptr_) {
            if (rhs.it_ != rhs.ptr_->end()) return false;
        }

        return true;
    }

    friend bool operator!=(pointer_container_iterator const & lhs, pointer_container_iterator const & rhs)
    {
        return !(lhs == rhs);
    }

    reference operator*() const
    {
        return *it_;
    }

    pointer operator->() const
    {
        return &*it_;
    }

    pointer_container_iterator & operator++()
    {
        ++it_;
        return *this;
    }

    pointer_container_iterator operator++(int)
    {
        pointer_container_iterator it = *this;
        ++it_;
        return it;
    }
    
private:
    managed_ptr ptr_;
    iterator it_;
};

} } }
