/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <limits>

namespace acqua { namespace container { namespace detail {

template <typename K, typename V, typename H, typename P, typename A>
class lru_map_impl
{
public:
    using key_type = K;
    using mapped_type = V;
    using value_type = std::pair<K const, V>;
    using size_type = std::size_t;
    using allocator_type = A;

    class node_base
    {
        friend lru_map_impl;

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

        void replace(value_type const & value)
        {
            const_cast<K &>(value_.first) = value.first;
            value_.second = value.second;
        }

        void replace(value_type && value)
        {
            const_cast<K &>(value_.first) = std::move(value.first);
            value_.second = std::move(value.second);
        }

        friend bool operator==(node_base const & lhs, node_base const & rhs)
        {
            return P()(lhs.value_.first, rhs.value_.first);
        }

        friend size_type hash_value(node_base const & rhs)
        {
            return H()(rhs.value_.first);
        }
    };

    struct hasher
    {
        size_type operator()(key_type const & rhs) const
        {
            return H()(rhs);
        }

        size_type operator()(value_type const & rhs) const
        {
            return H()(rhs.first);
        }
    };

    struct equal_to
    {
        bool operator()(key_type const & lhs, node_base const & rhs) const
        {
            return P()(lhs, rhs.value_.first);
        }

        bool operator()(value_type const & lhs, node_base const & rhs) const
        {
            return P()(lhs.first, rhs.value_.first);
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
};

} } }
