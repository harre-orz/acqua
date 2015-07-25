#pragma once

#include <string>

namespace acqua { namespace text { namespace adapted {

template <typename T>
class json_adaptor {};

template <typename CharT>
class json_adaptor< std::basic_string<CharT> >
{
    using self_type = std::basic_string<CharT>;
    self_type & self_;

public:
    explicit json_adaptor(self_type & self)
        : self_(self) {}

    void data(self_type & val)
    {
        self_ = val;
    }
};

} } }
