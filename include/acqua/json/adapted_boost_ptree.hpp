/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <acqua/json/adapted_proto.hpp>

namespace acqua { namespace json {

template <typename Key, typename Data, typename KeyCompare>
class adapted< boost::property_tree::basic_ptree<Key, Data, KeyCompare> >
{
    using self_type = boost::property_tree::basic_ptree<Key, Data, KeyCompare>;
    self_type & self_;

public:
    using char_type = typename Key::value_type;

    explicit adapted(self_type & self) : self_(self) {}

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

} }
