#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/email/message.hpp>
#include <boost/system/system_error.hpp>

namespace acqua { namespace email {

template <typename String>
auto basic_message<String>::set_payload() -> streambuf_type *
{
    streambuf_.reset(new stringbuf_type());
    return streambuf_.get();
}


template <typename String>
auto basic_message<String>::set_payload(std::string const & filename,boost::system::error_code & ec,  bool overwrite) -> streambuf_type *
{
    auto * filebuf = new filebuf_type();
    std::unique_ptr<streambuf_type> streambuf(filebuf);
    if (overwrite) {
        filebuf->open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        if (!filebuf->is_open()) {
            ec = make_error_code(boost::system::errc::no_such_file_or_directory);
            return nullptr;
        }
        filebuf->close();
    }
    filebuf->open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::app);
    if (!filebuf->is_open()) {
        ec = make_error_code(boost::system::errc::no_such_file_or_directory);
        return nullptr;
    }
    streambuf_ = std::move(streambuf);
    return streambuf_.get();
}


template <typename String>
auto basic_message<String>::set_payload(std::string const & filename, bool overwrite) -> streambuf_type *
{
    boost::system::error_code ec;
    set_payload(filename, ec, overwrite);
    if (ec) throw boost::system::system_error(ec);
    return streambuf_.get();
}


template <typename String>
basic_message<String>::operator streambuf_type *()
{
    return get_payload() ?: set_payload();
}


template <typename String>
auto basic_message<String>::str() const -> value_type
{
    if (auto * sbuf = dynamic_cast<stringbuf_type *>(streambuf_.get())) {
        return sbuf->str();
    }

    if (auto * fbuf = dynamic_cast<filebuf_type *>(streambuf_.get())) {
        std::basic_ostringstream<char_type, traits_type, allocator_type> oss;
        fbuf->pubseekpos(0, std::ios_base::in);
        oss << fbuf;
        return oss.str();
    }

    return value_type();
}


template <typename String>
bool basic_message<String>::save_as(std::string const & filename, boost::system::error_code & ec)
{
    std::basic_fstream<char_type, traits_type> ofs(filename);
    if (streambuf_) {
        streambuf_->pubseekpos(0, std::ios_base::in);
        ofs << &*streambuf_;
        if (ofs.good())
            return true;
    }

    ec = make_error_code(boost::system::errc::no_such_file_or_directory);
    return false;
}


template <typename String>
bool basic_message<String>::save_as(std::string const & filename)
{
    boost::system::error_code ec;
    return save_as(filename, ec);
}

} }
