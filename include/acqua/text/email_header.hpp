#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/operators.hpp>
#include <acqua/container/sequenced_map.hpp>

namespace acqua { namespace text {

/*!
  メールヘッダー用のデータ構造.

 */
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
        : private boost::totally_ordered<disposition, data_type>
    {
    private:
        using char_type = typename String::value_type;
        using traits_type = typename String::traits_type;
        using ostream_type = std::basic_ostream<char_type, traits_type>;
        using map_type = acqua::container::sequenced_map<key_type, data_type, iequal_to>;

    public: // data operations
        disposition & operator=(data_type const & data)
        {
            assign(data);
            return *this;
        }

        friend bool operator==(disposition const & lhs, key_type const & rhs) { return lhs.data_ == rhs; }
        friend bool operator<(disposition const & lhs, key_type const & rhs) { return lhs.data_ < rhs; }
        operator data_type &() { data_; }
        operator data_type const &() const { return data_; }

        data_type & str() { return data_; }
        data_type const & str() const { return data_; }
        void assign(data_type const & data) { data_.assign(data); }
        void assign(char_type const * str, std::size_t size) { data_.assign(str, size); }
        template <typename It> void assign(It beg, It end) { data_.assign(beg, end); }
        void append(data_type const & data) { data_.append(data); }
        void append(char_type const * str, std::size_t size) { data_.append(str, size); }
        template <typename It> void append(It beg, It end) { data_.append(beg, end); }

        friend ostream_type & operator<<(ostream_type & os, disposition const & rhs)
        {
            os << rhs.data_;
            return os;
        }

    public: // map operations
        using size_type = typename map_type::size_type;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

    public:
        bool empty() const { return map_.empty(); }
        size_type size() const { return map_.size(); }
        void clear() { map_.clear(); }
        iterator begin() { return map_.begin(); }
        const_iterator begin() const { return map_.begin(); }
        iterator end() { return map_.end(); }
        const_iterator end() const { return map_.end(); }
        iterator find(key_type const & name) { return map_.find(name); }
        const_iterator find(key_type const & name) const { return map_.find(name); }
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

    bool empty() const { return multimap_.empty(); }
    size_type size() const { return multimap_.size(); }
    void clear() { multimap_.clear(); }
    iterator begin() { return multimap_.begin(); }
    const_iterator begin() const { return multimap_.begin(); }
    iterator end() { return multimap_.end(); }
    const_iterator end() const { return multimap_.end(); }
    iterator find(key_type const & name) { return multimap_.find(name); }
    const_iterator find(key_type const & name) const { return multimap_.find(name); }
    disposition & operator[](key_type const & name) { return multimap_[name]; }

    void dump(std::ostream & os) const
    {
        for(auto const & a : multimap_) {
            os << a.first << ':' << ' ' << a.second;
            for(auto & b : a.second)
                os << ' ' << b.first << '=' << '"' << b.second << '"';
            os << std::endl;
        }
    }

private:
    mutable multimap_type multimap_;
};

} }
