/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iterator>

namespace acqua { namespace container { namespace detail {

/*!
  コンテナ T のポインタをイテレータに内包したデータクラス.

  イテレータの end を取得できない場合でも it != pointer_container_iterator() で判定できる
 */
template <typename T, typename Iter, typename ManagedPtr>
class pointer_container_iterator
    : public std::iterator<std::forward_iterator_tag, typename Iter::value_type>
{
public:
    using iterator = Iter;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;
    using managed_ptr = ManagedPtr;

public:
    pointer_container_iterator() = default;
    pointer_container_iterator(pointer_container_iterator const & rhs) = default;
    pointer_container_iterator(pointer_container_iterator && rhs) = default;

    pointer_container_iterator(managed_ptr const & ptr)
        : ptr_(ptr)
        , it_(ptr_->begin())
    {
    }

    pointer_container_iterator(iterator it, managed_ptr const & ptr)
        : ptr_(ptr)
        , it_(it)
    {
    }

    pointer_container_iterator(managed_ptr && ptr)
        : ptr_(std::move(ptr))
        , it_(ptr_->begin())
    {
    }

    pointer_container_iterator(iterator it, managed_ptr && ptr)
        : ptr_(std::move(ptr))
        , it_(it)
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

    pointer get() const
    {
        return ptr_.get();
    }

    void release()
    {
        ptr_ = nullptr;
    }

private:
    managed_ptr ptr_;
    iterator it_;
};

} } }
