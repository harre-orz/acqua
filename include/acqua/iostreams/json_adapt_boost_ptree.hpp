/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <acqua/iostreams/json_adapt.hpp>

namespace acqua { namespace iostreams {

template <typename Key, typename Data, typename KeyCompare>
struct json_adapt< boost::property_tree::basic_ptree<Key, Data, KeyCompare> >
{
public:
    using value_type = boost::property_tree::basic_ptree<Key, Data, KeyCompare>;
    using char_type = typename Key::value_type;

public:
    explicit json_adapt(value_type & self) : value_(self) {}

    void data(std::nullptr_t const &) const
    {
        value_.data() = "null";
    }

    void data(bool const & flag) const
    {
        value_.data() = (flag ? "true" : "false");
    }

    void data(long val) const
    {
        value_.data() = std::to_string(val);
    }

    void data(double const & val) const
    {
        value_.data() = std::to_string(val);
    }

    void data(Key const & val) const
    {
        value_.data() = val;
    }

    value_type & add_child(int) const
    {
        return value_.push_back(typename value_type::value_type("", value_type()))->second;
    }

    value_type & add_child(Key const & key) const
    {
        return value_.push_back(typename value_type::value_type(key, value_type()))->second;
    }

private:
    value_type & value_;
};

} }
