#pragma once

#include <boost/algorithm/string.hpp>
#include <acqua/container/sequenced_map.hpp>

namespace acqua { namespace text {

template <typename String>
class email_header
{
public:
    using value_type = String;
    using size_type = typename value_type::size_type;

private:
    struct iequal_to
    {
        bool operator()(value_type const & lhs, value_type const & rhs) const
        {
            return boost::iequals(lhs, rhs);
        }
    };

    using map_type = acqua::container::sequenced_map<
        value_type,
        std::pair<
            value_type,
            acqua::container::sequenced_map<
                value_type,
                value_type,
                iequal_to
                >
            >,
        iequal_to
        >;

private:
    static value_type const & s_empty()
    {
        static value_type empty;
        return empty;
    }

public:
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;
    using param_iterator = typename map_type::mapped_type::second_type::iterator;
    using const_param_iterator = typename map_type::mapped_type::second_type::const_iterator;

public:
    value_type & operator()(value_type const & name)
    {
        return header_[name].first;
    }

    value_type const & operator()(value_type const & name) const
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second : s_empty());
    }

    value_type const & operator()(value_type const & name, value_type const & param) const
    {
        auto it1 = header_.find(name);
        if (it1 == header_.end())
            return empty();
        auto it2 = it1->second.find(param);
        if (it2 == it1->second.end())
            return empty();
        return it2->second;
    }

    value_type & operator()(value_type const & name, value_type const & param)
    {
        return header_[name].second[param];
    }

    bool empty() const
    {
        return header_.empty();
    }

    bool empty(value_type const & name) const
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.empty() : true);
    }

    size_type size() const
    {
        return header_.size();
    }

    size_type size(value_type const & name) const
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.size() + 1 : 0);
    }

    void clear()
    {
        header_.clear();
    }

    void clear(value_type const & name)
    {
        auto it = header_.find(name);
        if (it != header_.end())
            header_.erase(it);
    }

    iterator begin()
    {
        return header_.begin();
    }

    const_iterator begin() const
    {
        return header_.begin();
    }

    param_iterator begin(value_type const & name)
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.begin() : iterator());
    }

    const_param_iterator begin(value_type const & name) const
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.begin() : const_iterator());
    }

    iterator end()
    {
        return header_.end();
    }

    const_iterator end() const
    {
        return header_.end();
    }

    param_iterator end(value_type const & name)
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.end() : iterator());
    }

    const_param_iterator end(value_type const & name) const
    {
        auto it = header_.find(name);
        return (it != header_.end() ? it->second.end() : const_iterator());
    }

    void dump(std::ostream & os) const
    {
        for(auto it = header_.begin(); it != header_.end(); ++it) {
            os << it->first << ':' << ' ' << it->second.first << std::endl;
        }
    }

private:
    mutable map_type header_;
};

} }
