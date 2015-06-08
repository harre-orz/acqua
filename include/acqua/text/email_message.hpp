#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <acqua/text/email_header.hpp>

namespace acqua { namespace text {

template <typename String>
class email_message
    : public std::basic_iostream<typename String::value_type, typename String::traits_type>
{
    using base_type = std::basic_iostream<typename String::value_type, typename String::traits_type>;

public:
    using char_type = typename String::value_type;
    using traits_type = typename String::traits_type;
    using allocator_type = typename String::allocator_type;
    using buffer_type = std::basic_streambuf<char_type, traits_type>;
    using istream_type = std::basic_istream<char_type, traits_type>;
    using ostream_type = std::basic_ostream<char_type, traits_type>;

public:
    using value_type = String;
    using header_type = email_header<String>;

public:
    email_message()
        : buffer_(new std::basic_stringbuf<char_type, traits_type, allocator_type>())
    {
        base_type::rdbuf(&*buffer_);
    }

    void open(value_type const & filename)
    {
        buffer_.reset(new std::basic_filebuf<char_type, traits_type>(filename));
        base_type::rdbuf(&*buffer_);
    }

    header_type header;

private:
    std::unique_ptr<buffer_type> buffer_;
};

} }
