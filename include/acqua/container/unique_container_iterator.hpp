#pragma once

#include <type_traits>
#include <memory>

namespace acqua { namespace container {

template <typename T, typename UniquePtr = std::unique_ptr<T> >
class unique_container_iterator
    : public std::iterator<
        typename std::conditional<
            std::is_const<T>::value,
            typename T::const_iterator,
            typename T::iterator
            >::type::iterator_category,
        typename T::value_type
    >
{
public:
    using unique_ptr = UniquePtr;
    using value_type = typename T::value_type;
    using iterator = typename T::iterator;
    using const_iterator = typename T::const_iterator;
    using specified_iterator = typename std::conditional<std::is_const<T>::value, const_iterator, iterator>::type;
    using specified_value_type = typename std::conditional<std::is_const<T>::value, value_type const, value_type>::type;

    unique_container_iterator() noexcept = default;
    unique_container_iterator(unique_container_iterator const &) noexcept = default;
    unique_container_iterator(unique_container_iterator &&) noexcept = default;

    unique_container_iterator(unique_ptr && ptr) noexcept
        : ptr_(std::move(ptr)), it_(ptr_->begin()) {}

    template <typename It>
    unique_container_iterator(It it, unique_ptr && ptr) noexcept
        : ptr_(std::move(ptr)), it_(it) {}

    unique_container_iterator(T * ptr) noexcept
        : ptr_(ptr), it_(ptr_->begin()) {}

    template <typename It>
    unique_container_iterator(It it, T * ptr) noexcept
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

    bool operator==(unique_container_iterator const & rhs) const noexcept
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

    bool operator!=(unique_container_iterator const & rhs) const noexcept
    {
        return !operator==(rhs);
    }

    unique_container_iterator & operator++() noexcept
    {
        ++it_;
        return *this;
    }

private:
    unique_ptr ptr_;
    specified_iterator it_;
};

} }
