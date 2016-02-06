#pragma once

#include <memory>
#include <boost/system/error_code.hpp>
#include <boost/iostreams/categories.hpp>

namespace acqua { namespace email {

template <typename String>
class basic_email;


template <typename String>
class basic_email_parser
{
    struct impl;

public:
    using char_type = char;
    using category = boost::iostreams::sink_tag;

    explicit basic_email_parser(basic_email<String> & email);

    explicit operator bool() const
    {
        return !error_;
    }

    std::streamsize write(char_type const * s, std::streamsize n);

private:
    std::shared_ptr<impl> impl_;
    boost::system::error_code error_;
};

using email_parser = basic_email_parser<std::string>;
using wemail_parser = basic_email_parser<std::wstring>;

} }

#include <acqua/email/impl/email_parser.ipp>
