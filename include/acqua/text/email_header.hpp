#pragma once

#include <boost/algorithm/string.hpp>
#include <acqua/container/sequenced_map.hpp>

namespace acqua { namespace text {

template <typename String>
class email_header
{
public:
    using key_type = String;
    using data_type = String;

private:
    struct iequal_to
    {
        bool operator()(key_type const & lhs, key_type const & rhs) const
        {
            return boost::algorithm::iequals(lhs, rhs);
        }
    };

public:
    class disposition
    {
    private:
        using char_type = typename String::value_type;
        using traits_type = typename String::traits_type;
        using ostream_type = std::basic_ostream<char_type, traits_type>;
        using map_type = acqua::container::sequenced_map<key_type, data_type, iequal_to>;

    public: // data operations
        disposition & operator=(data_type const & data)
        {
            data_ = data;
            return *this;
        }

        friend bool operator==(disposition const & lhs, key_type const & rhs)
        {
            return lhs.data_ == rhs;
        }

        friend ostream_type & operator<<(ostream_type & os, disposition const & rhs)
        {
            os << rhs.data_;
            return os;
        }

    public: // map operations
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

        iterator begin() { return map_.begin(); }
        const_iterator begin() const { return map_.begin(); }
        iterator end() { return map_.end(); }
        const_iterator end() const { return map_.end(); }

        data_type & operator[](key_type const & name)
        {
            return map_[name];
        }

    private:
        data_type data_;
        map_type map_;
    };

private:
    using multimap_type = acqua::container::sequenced_multimap<key_type, disposition, iequal_to>;

public:
    using size_type = typename multimap_type::size_type;
    using iterator = typename multimap_type::iterator;
    using const_iterator = typename multimap_type::const_iterator;

    iterator begin()
    {
        return multimap_.begin();
    }

    const_iterator begin() const
    {
        return multimap_.begin();
    }

    iterator end()
    {
        return multimap_.end();
    }

    const_iterator end() const
    {
        return multimap_.end();
    }

    disposition & operator[](key_type const & name)
    {
        return multimap_[name];
    }

private:
    multimap_type multimap_;
};

} }
