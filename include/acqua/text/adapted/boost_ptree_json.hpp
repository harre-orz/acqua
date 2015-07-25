#pragma once

#include <boost/property_tree/ptree.hpp>
#include <acqua/text/adapted/json_adaptor.hpp>

namespace acqua { namespace text { namespace adapted {

template <typename Key, typename Data, typename KeyCompare>
class json_adaptor< boost::property_tree::basic_ptree<Key, Data, KeyCompare> >
{
    using self_type = boost::property_tree::basic_ptree<Key, Data, KeyCompare>;
    self_type & self_;

public:
    explicit json_adaptor(self_type & self) : self_(self) {}

    void data(std::nullptr_t const &) const
    {
        self_.data() = "null";
    }

    void data(bool const & flag) const
    {
        self_.data() = (flag ? "true" : "false");
    }

    void data(long val) const
    {
        self_.data() = std::to_string(val);
    }

    void data(double const & val) const
    {
        self_.data() = std::to_string(val);
    }

    void data(Key const & val) const
    {
        self_.data() = val;
    }

    self_type & add_child(int index) const
    {
        return self_.push_back(typename self_type::value_type(std::to_string(index), self_type()))->second;
    }

    self_type & add_child(Key const & key) const
    {
        return self_.push_back(typename self_type::value_type(key, self_type()))->second;
    }
};

} } }
