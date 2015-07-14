#pragma once

#include <iostream>
#include <iterator>
#include <stack>
#include <boost/operators.hpp>

namespace acqua { namespace container {

template <
    typename T,
    typename It,
    It (*begin)(T &) = &std::begin<T>,
    It (*end)(T &) = &std::end<T>,
    typename Stack = std::stack<It>
    >
class preordered_recursive_iterator
    : public std::iterator<std::forward_iterator_tag, T>
    , private boost::equality_comparable<T>
{
public:
    preordered_recursive_iterator()
        : root_(nullptr)
    {
    }

    preordered_recursive_iterator(T & root)
        : root_(&root)
    {
    }

    friend bool operator==(preordered_recursive_iterator const & lhs, preordered_recursive_iterator const & rhs)
    {
        return (lhs.depth_.empty()) ? (rhs.depth_.empty() ? lhs.root_ == rhs.root_ : false)
            :  (rhs.depth_.empty()) ? false
            :  lhs.depth_.top() == rhs.depth_.top();
    }

    friend bool operator==(preordered_recursive_iterator const & lhs, It rhs)
    {
        return (lhs.depth_.empty()) ? false
            :  lhs.depth_.top() == rhs;
    }

    T & operator*() const
    {
        return depth_.empty() ? *root_ : *depth_.top();
    }

    T * operator->() const
    {
        return depth_.empty() ? &*root_ : &*depth_.top();
    }

    std::size_t depth() const
    {
        return depth_.size();
    }

    preordered_recursive_iterator & operator++()
    {
        It it = (depth_.empty())
            ? (*begin)(*root_)
            : (*begin)(*depth_.top());
        incr(it);
        return *this;
    }

    void incr(It & it)
    {
        if (depth_.empty()) {
            if (it == (*end)(*root_)) {
                root_ = nullptr;
            } else {
                depth_.push(it);
            }
        } else {
            if (it == (*end)(*depth_.top())) {
                it = ++depth_.top();
                depth_.pop();
                incr(it);
            } else {
                depth_.push(it);
            }
        }
    }

private:
    T * root_;
    Stack depth_;
};

} }
