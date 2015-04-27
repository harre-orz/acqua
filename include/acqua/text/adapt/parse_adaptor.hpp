#pragma once

#include <string>

namespace acqua { namespace text { namespace adapt {

template <typename T>
class parse_adaptor {};

template <typename CharT>
class parse_adaptor< std::basic_string<CharT> >
{
    using self_type = std::basic_string<CharT>;
    self_type & self_;

public:
    explicit parse_adaptor(self_type & self)
        : self_(self) {}

    void parse_value(self_type & val)
    {
        self_ = val;
    }
};

} } }
