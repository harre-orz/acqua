#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <boost/iterator/iterator_adaptor.hpp>
#include <stdexcept>
#include <algorithm>

namespace acqua { namespace container { namespace detail {

template <typename Derived,
          typename Key, typename Value, typename Pred, typename Alloc,
          template <typename T, typename A> class Container>
class sequenced_map_base
    : private Pred
    , private Alloc
{
protected:
    using base_type = sequenced_map_base<Derived, Key, Value, Pred, Alloc, Container>;

    ~sequenced_map_base() {}

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<Key const, Value>;
    using key_equal = Pred;
    using allocator_type = Alloc;

private:
    struct wrapped_value_type : std::pair<key_type, mapped_type>
    {
        template <typename... Args>
        wrapped_value_type(Args... args) : std::pair<key_type, mapped_type>(args...) {}
        operator value_type &() { return *reinterpret_cast<value_type *>(this); }
        operator value_type &() const { return *reinterpret_cast<value_type *>(const_cast<wrapped_value_type *>(this)); }
    };
    using data_type = Container<wrapped_value_type, typename Alloc::template rebind<wrapped_value_type>::other>;

public:
    explicit sequenced_map_base(key_equal const & key_eq, allocator_type const & alloc)
        : key_equal(key_eq), allocator_type(alloc) {}

    struct iterator : boost::iterator_adaptor<iterator, typename data_type::iterator, value_type>
    {
        iterator() {}

        iterator(typename data_type::iterator it)
            : boost::iterator_adaptor<iterator, typename data_type::iterator, value_type>(it) {}
    };

    struct const_iterator : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator, value_type>
    {
        const_iterator() {}

        const_iterator(typename data_type::const_iterator it)
            : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator, value_type>(it) {}

        const_iterator(iterator it)
            : boost::iterator_adaptor<const_iterator, typename data_type::const_iterator, value_type>(it.base()) {}
    };

    struct reverse_iterator : boost::iterator_adaptor<reverse_iterator, typename data_type::reverse_iterator, value_type>
    {
        reverse_iterator() {}

        reverse_iterator(typename data_type::reverse_iterator it)
            : boost::iterator_adaptor<reverse_iterator, typename data_type::reverse_iterator, value_type>(it) {}
    };

    struct const_reverse_iterator : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator, value_type>
    {
        const_reverse_iterator() {}

        const_reverse_iterator(typename data_type::const_reverse_iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator, value_type>(it) {}

        const_reverse_iterator(reverse_iterator it)
            : boost::iterator_adaptor<const_reverse_iterator, typename data_type::const_reverse_iterator, value_type>(it.base()) {}
    };

    using size_type = typename data_type::size_type;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;

public:
    key_equal key_eq() const { return *static_cast<key_equal const *>(this); }
    allocator_type get_allocator() const { return *static_cast<allocator_type const *>(this); }
    bool empty() const { return data_.empty(); }
    size_type size() const { return data_.size(); }
    iterator begin() { return data_.begin(); }
    const_iterator begin() const { return data_.begin(); }
    iterator end() { return data_.end(); }
    const_iterator end() const { return data_.end(); }
    reverse_iterator rbegin() { return data_.rbegin(); }
    const_reverse_iterator rbegin() const { return data_.rbegin(); }
    reverse_iterator rend() { return data_.rend(); }
    const_reverse_iterator rend() const { return data_.rend(); }

    void clear() { data_.clear(); }
    mapped_type & at(key_type const & key)
    {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("sequenced_map::at");
        return it->second;
    }

    mapped_type const & at(key_type const & key) const
    {
        auto it = find(key);
        if (it == end())
            throw std::out_of_range("sequenced_map::at");
        return it->second;
    }
    iterator find(key_type const & key)
    {
        return std::find_if(data_.begin(), data_.end(), [&key](auto const & elem) { return key_equal()(elem.first, key); });
    }
    const_iterator find(key_type const & key) const
    {
        return std::find_if(data_.begin(), data_.end(), [&key](auto const & elem) { return key_equal()(elem.first, key); });
    }
    iterator erase(const_iterator it) { return data_.erase(it.base()); }
    iterator erase(const_iterator beg, const_iterator end) { return data_.erase(beg.base(), end.base()); }

    template <typename It>
    iterator assign(It beg, It end)
    {
        clear();
        return insert(beg, end);
    }

    template <typename It>
    iterator insert(It beg, It end)
    {
        iterator it;
        while(beg != end) {
            it = static_cast<Derived *>(this)->insert(const_iterator(), *beg);
            ++beg;
        }
        return ++it;
    }

protected:
    iterator find(value_type const & val) { return find(val.first); }
    const_iterator find(value_type const & val) const { return find(val.first); }
    iterator find(key_type const & key, mapped_type const &) { return find(key); }
    const_iterator find(key_type const & key, mapped_type const &) const { return find(key); }
    data_type data_;
};

} } }
