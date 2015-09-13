/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <acqua/email/email_fwd.hpp>
#include <acqua/email/basic_message.hpp>
#include <acqua/email/basic_address.hpp>
#include <acqua/email/email_traits.hpp>
#include <acqua/email/utils/parse_address.hpp>

namespace acqua { namespace email {

template <
    typename String,
    typename Traits = email_traits<String>
    >
class basic_email
{
public:
    enum encoding_type { noop, ascii, qprint, base64 };

    using value_type = String;
    using message = basic_message<value_type>;
    using address = basic_address<value_type>;

    using char_type = typename message::char_type;
    using traits_type = typename message::traits_type;
    using allocator_type = typename message::allocator_type;
    using streambuf_type = typename message::streambuf_type;
    using istream_type = std::basic_istream<char_type, traits_type>;
    using ostream_type = std::basic_ostream<char_type, traits_type>;
    using disposition = typename message::disposition;
    using size_type = typename message::size_type;
    using iterator = typename message::iterator;
    using const_iterator = typename message::const_iterator;
    using subpart_type = typename message::subpart_type;
    using subpart_iterator = typename message::subpart_iterator;
    using const_subpart_iterator = typename message::const_subpart_iterator;
    using recursive_iterator = typename message::recursive_iterator;
    using const_recursive_iterator = typename message::const_recursive_iterator;

    basic_email(std::string const & charset = "utf-8", encoding_type enctype = qprint)
        : impl_(new message())
    {
        auto & contenttype = (*this)["Content-Type"];
        contenttype = "text/plain";
        contenttype["charset"] = charset;
        switch(enctype) {
            case noop:
                (*this)["Content-Transfer-Encoding"] = "8bit";
                break;
            case ascii:
                (*this)["Content-Transfer-Encoding"] = "7bit";
                break;
            case qprint:
                (*this)["Content-Transfer-Encoding"] = "quoted-printable";
                break;
            case base64:
                (*this)["Content-Transfer-Encoding"] = "base64";
                break;
        }
    }

public:
    bool empty() const { return impl_->empty(); }
    size_type size() const { return impl_->size(); }
    void clear() { impl_->clear(); }
    iterator begin() { return impl_->begin(); }
    const_iterator begin() const { return impl_->begin(); }
    iterator end() { return impl_->end(); }
    const_iterator end() const { return impl_->end(); }
    iterator find(value_type const & name) { return impl_->find(name); }
    const_iterator find(value_type const & name) const { return impl_->find(name); }
    disposition & operator[](value_type const & name) { return impl_->operator[](name); }
    message & add_subpart() { return impl_->add_subpart(); }
    recursive_iterator recbegin() { return impl_->recbegin(); }
    const_recursive_iterator recbegin() const { return impl_->recbegin(); }
    recursive_iterator recend() { return impl_->recend(); }
    const_recursive_iterator recend() const { return impl_->recend(); }

public:
    operator message &() { return (*impl_); }

    void attach(istream_type & is, value_type const & content = value_type())
    {
        using std::swap;

        std::unique_ptr<message> main;
        if (!impl_->has_subpart()) {
            // impl_ が単体だったら、マルチパート化させる必要がある
            main.reset(new message());
            auto & contenttype = (*main)["Content-Type"];
            contenttype = "multipart/related";
            //contenttype["boundary"] = ""  // バウンダリの指定がない場合は、メール文生成時に自動生成される
            swap(impl_, main);
        }

        try {
            auto & subpart = impl_->add_subpart();
            auto & contenttype = subpart["Content-Type"];
            contenttype = content;
            subpart["Content-Transfer-Encoding"] = "base64";

            ostream_type os(subpart);
            std::copy(std::istreambuf_iterator<char_type>(is), std::istreambuf_iterator<char_type>(), std::ostreambuf_iterator<char_type>(os));
        } catch(...) {
            if (main)
                swap(impl_, main);
            throw;
        }
    }

    void attach(std::string const & filename, value_type const & content = value_type())
    {
        std::basic_ifstream<char_type, traits_type> ifs(filename);
        if (!ifs.good())
            throw std::runtime_error("file open error");
        attach(ifs, content);
    }

private:
    std::unique_ptr<message> impl_;
};

} }
