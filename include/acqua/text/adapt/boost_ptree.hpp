#pragma once

#include <acqua/text/adapt/parse_adaptor.hpp>
#include <boost/property_tree/ptree.hpp>

namespace acqua { namespace text { namespace adapt {

template <typename Key, typename Data, typename KeyCompare>
class parse_adaptor< boost::property_tree::basic_ptree<Key, Data, KeyCompare> >
{
    using self_type = boost::property_tree::basic_ptree<Key, Data, KeyCompare>;
    self_type & self_;

public:
    explicit parse_adaptor(self_type & self) : self_(self) {}

    void parse_value(std::nullptr_t const &) const
    {
        self_.data() = "null";
    }

    void parse_value(bool const & flag) const
    {
        self_.data() = (flag ? "true" : "false");
    }

    void parse_value(long val) const
    {
        self_.data() = std::to_string(val);
    }

    void parse_value(double const & val) const
    {
        self_.data() = std::to_string(val);
    }

    void parse_value(Key const & val) const
    {
        self_.data() = val;
    }

    self_type & parse_child(int index) const
    {
        return self_.push_back(typename self_type::value_type(std::to_string(index), self_type()))->second;
    }

    self_type & parse_child(Key const & key) const
    {
        return self_.push_back(typename self_type::value_type(key, self_type()))->second;
    }
};

} } }
