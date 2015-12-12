/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/operators.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <acqua/container/sequenced_map.hpp>

namespace acqua { namespace email {

/*!
  メールヘッダークラス.
 */
template <typename String>
class basic_headers
{
public:
    using value_type = String;
    class disposition;

private:
    struct is_iequal
    {
        bool operator()(String const & lhs, String const & rhs) const
        {
            return boost::algorithm::iequals(lhs, rhs);
        }
    };

    using multimap_type = acqua::container::sequenced_multimap<value_type, disposition, is_iequal>;
    using char_type = typename value_type::value_type;
    using traits_type = typename value_type::traits_type;
    using ostream_type = std::basic_ostream<char_type, traits_type>;

public:
    using size_type = typename multimap_type::size_type;
    using iterator = typename multimap_type::iterator;
    using const_iterator = typename multimap_type::const_iterator;
    using reverse_iterator = typename multimap_type::reverse_iterator;
    using const_reverse_iterator = typename multimap_type::const_reverse_iterator;

public:
    bool empty() const
    {
        return params_.empty();
    }

    size_type size() const
    {
        return params_.size();
    }

    iterator begin()
    {
        return params_.begin();
    }

    const_iterator begin() const
    {
        return params_.begin();
    }

    iterator end()
    {
        return params_.end();
    }

    const_iterator end() const
    {
        return params_.end();
    }

    iterator find(value_type const & name)
    {
        return params_.find(name);
    }

    const_iterator find(value_type const & name) const
    {
        return params_.find(name);
    }

    iterator erase(const_iterator it)
    {
        return params_.erase(it);
    }

    iterator erase(const_iterator beg, const_iterator end)
    {
        return params_.erase(beg, end);
    }

    disposition & at(value_type const & name)
    {
        return params_.at(name);
    }

    disposition & operator[](value_type const & name)
    {
        auto it = params_.find(name);
        if (it == params_.end())
            it = params_.emplace_hint(params_.begin(), name, disposition());
        return it->second;
    }

    void dump(ostream_type & os) const
    {
        for(auto && a : params_) {
            os << a.first << ':' << ' ' << a.second;
            for(auto && b : a.second) {
                os << ' ' << b.first << '=' << '"' << b.second << '"';
            }
            os << std::endl;
        }
    }

private:
    multimap_type params_;
};


template <typename String>
class basic_headers<String>::disposition
    : private boost::totally_ordered<disposition, value_type>
{
private:
    using map_type = acqua::container::sequenced_map<value_type, value_type, is_iequal>;

public:
    using size_type = typename map_type::size_type;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;
    using reverse_iterator = typename map_type::reverse_iterator;
    using const_reverse_iterator = typename map_type::reverse_iterator;

public:
    value_type & str()
    {
        return data_;
    }

    value_type const & str() const
    {
        return data_;
    }

    template <typename... Args>
    void assign(Args&&... args)
    {
        data_.assign(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void append(Args&&... args)
    {
        data_.append(std::forward<Args>(args)...);
    }

    bool empty() const
    {
        return params_.empty();
    }

    size_type size() const
    {
        return params_.size();
    }

    void clear()
    {
        params_.clear();
    }

    iterator begin()
    {
        return params_.begin();
    }

    const_iterator begin() const
    {
        return params_.begin();
    }

    iterator end()
    {
        return params_.end();
    }

    const_iterator end() const
    {
        return params_.end();
    }

    reverse_iterator rbegin()
    {
        return params_.rbegin();
    }

    const_reverse_iterator rbegin() const
    {
        return params_.rbegin();
    }

    reverse_iterator rend()
    {
        return params_.rend();
    }

    const_reverse_iterator rend() const
    {
        return params_.rend();
    }

    const_iterator cbegin() const
    {
        return params_.cbegin();
    }

    const_iterator cend() const
    {
        return params_.cend();
    }

    const_reverse_iterator crbegin() const
    {
        return params_.crbegin();
    }

    const_reverse_iterator crend() const
    {
        return params_.crend();
    }

    iterator find(value_type const & name)
    {
        return params_.find(name);
    }

    const_iterator find(value_type const & name) const
    {
        return params_.find(name);
    }

    value_type & at(value_type const & name)
    {
        return params_.at(name);
    }

    value_type & operator[](value_type const & name)
    {
        auto it = params_.find(name);
        if (it == params_.end())
            it = params_.emplace_hint(params_.begin(), name, value_type());
        return it->second;
    }

    template <typename T>
    disposition & operator=(T && t)
    {
        assign(std::forward<T>(t));
        return *this;
    }

    template <typename T>
    disposition & operator+=(T && t)
    {
        append(std::forward<T>(t));
        return *this;
    }

    friend bool operator==(disposition const & lhs, value_type const & rhs)
    {
        return (lhs.data_ == rhs);
    }

    friend bool operator<(disposition const & lhs, value_type const & rhs)
    {
        return (lhs.data_ < rhs);
    }

    friend ostream_type & operator<<(ostream_type & os, disposition const & rhs)
    {
        os << rhs.data_;
        return os;
    }

private:
    value_type data_;
    map_type params_;
};

using headers = basic_headers<std::string>;
using wheaders = basic_headers<std::wstring>;

} }
