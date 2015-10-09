/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

namespace acqua { namespace container { namespace detail {

template <typename T, typename H, typename P, typename A, typename C>
class timed_lru_set_impl
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using allocator_type = A;
    using clock_type = C;
    using time_point_type = typename clock_type::time_point;
    using duration_type = typename clock_type::duration;

    class node_base
    {
        friend timed_lru_set_impl;

    protected:
        value_type value_;
        time_point_type expire_;

    public:
        explicit node_base(value_type const & value, timed_lru_set_impl const & impl)
            : value_(value)
            , expire_(clock_type::now() + impl.min_expire_)
        {
        }

        explicit node_base(value_type && value, timed_lru_set_impl const & impl)
            : value_(std::move(value))
            , expire_(clock_type::now() + impl.min_expire_)
        {
        }

        void replace(value_type const & value, timed_lru_set_impl const & impl)
        {
            const_cast<T &>(value_) = value;
            expire_ = clock_type::now() + impl.min_expire_;
        }

        void replace(value_type && value, timed_lru_set_impl const & impl)
        {
            const_cast<T &>(value_) = std::move(value);
            expire_ = clock_type::now() + impl.min_expire_;
        }

        friend bool operator==(node_base const & lhs, node_base const & rhs)
        {
            return P()(lhs.value_, rhs.value_);
        }

        friend size_type hash_value(node_base const & rhs)
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

        size_type operator()(node_base const & rhs) const
        {
            return H()(rhs.value_);
        }
    };

    struct equal_to
    {
        bool operator()(value_type const & lhs, node_base const & rhs) const
        {
            return P()(lhs, rhs.value_);
        }
    };

    size_type max_size() const
    {
        return max_size_;
    }

    void max_size(size_type size)
    {
        max_size_ = size;
    }

    duration_type const & expire() const
    {
        return min_expire_;
    }

    void expire(duration_type const & duration)
    {
        min_expire_ = duration;
    }

    template <typename Derived>
    bool is_limits(Derived const & t) const
    {
        return t.size() > max_size_ && (--t.end()).base()->expire_ < clock_type::now();
    }

private:
    size_type max_size_ = std::numeric_limits<size_type>::max();
    duration_type min_expire_ = duration_type::max();

protected:
    using key_type = T;
};

} } }
