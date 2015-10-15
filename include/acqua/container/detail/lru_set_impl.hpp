/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <limits>

namespace acqua { namespace container { namespace detail {

template <typename T, typename H, typename P, typename A>
class lru_set_impl
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = A;

    class node_base
    {
        friend lru_set_impl;

    protected:
        value_type value_;

    public:
        explicit node_base(value_type const & value)
            : value_(value)
        {
        }

        explicit node_base(value_type && value)
            : value_(std::move(value))
        {
        }

        void replace(value_type const & rhs)
        {
            const_cast<T &>(value_) = rhs;
        }

        void replace(value_type && rhs)
        {
            const_cast<T &>(value_) = std::move(rhs);
        }

        friend bool operator==(node_base const & lhs, node_base const & rhs)
        {
            return P()(lhs.value_, rhs.value_);
        }

        friend std::size_t hash_value(node_base const & rhs)
        {
            return H()(rhs.value_);
        }
    };

    struct hasher
    {
        size_type operator()(value_type const & rhs) const
        {
            return H()(rhs);
        }
    };

    struct equal_to
    {
        bool operator()(value_type const & lhs, node_base const & rhs) const
        {
            return P()(lhs, rhs.value_);
        }
    };

    size_type get_max_size() const
    {
        return max_size_;
    }


    void set_max_size(size_type size)
    {
        max_size_ = size;
    }

    template <typename Derived>
    bool is_limits(Derived const & t) const
    {
        return t.size() > max_size_;
    }

private:
    size_type max_size_ = std::numeric_limits<size_type>::max();

protected:
    using key_type = T;
};

} } }
