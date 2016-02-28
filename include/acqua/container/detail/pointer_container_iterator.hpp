#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
*/

#include <iterator>

namespace acqua { namespace container { namespace detail {

/*!
  コンテナ<T> のポインタをイテレータに内包したデータクラス.

  イテレータの end を取得できない場合でも it != pointer_container_iterator() で判定できる
*/
template <typename T, typename Iter, typename ManagedPtr>
class pointer_container_iterator
    : public std::iterator<std::forward_iterator_tag, T>
{
protected:
    using base_type = pointer_container_iterator;

public:
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using managed_ptr = ManagedPtr;
    using element_type = typename managed_ptr::element_type;

public:
    pointer_container_iterator() = default;

    pointer_container_iterator(pointer_container_iterator const & rhs) = default;

    pointer_container_iterator(pointer_container_iterator && rhs) = default;

    pointer_container_iterator(managed_ptr const & ptr);

    pointer_container_iterator(managed_ptr const & ptr, Iter it);

    pointer_container_iterator(managed_ptr && ptr);

    pointer_container_iterator(managed_ptr && ptr, Iter it);

    pointer_container_iterator(element_type * ptr);

    pointer_container_iterator(element_type * ptr, Iter it);

    pointer_container_iterator & operator=(pointer_container_iterator const & rhs) = default;

    pointer_container_iterator & operator=(pointer_container_iterator && rhs) = default;

    reference operator*() const;

    pointer operator->() const;

    pointer_container_iterator & operator++();

    pointer_container_iterator operator++(int);

    element_type * get() const;

    template <typename T_, typename Iter_, typename ManagedPtr_>
    friend bool operator==(pointer_container_iterator<T_, Iter_, ManagedPtr_> const & lhs, pointer_container_iterator<T_, Iter_, ManagedPtr_> const & rhs);

    template <typename T_, typename Iter_, typename ManagedPtr_>
    friend bool operator!=(pointer_container_iterator<T_, Iter_, ManagedPtr_> const & lhs, pointer_container_iterator<T_, Iter_, ManagedPtr_> const & rhs);

private:
    managed_ptr ptr_;
    Iter it_;
};

} } }

#include <acqua/container/detail/pointer_container_iterator.ipp>
