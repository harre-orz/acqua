/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/operators.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <acqua/container/sequenced_map.hpp>
#include <acqua/exception/throw_error.hpp>
#include <acqua/container/recursive_iterator.hpp>
#include <acqua/email/email_fwd.hpp>

namespace acqua { namespace email {

template <typename String>
class basic_message
{
public:
    using char_type = typename String::value_type;
    using traits_type = typename String::traits_type;
    using allocator_type = typename String::allocator_type;
    using value_type = String;
    using streambuf_type = std::basic_streambuf<char_type, traits_type>;
    using istream_type = std::basic_istream<char_type, traits_type>;
    using ostream_type = std::basic_ostream<char_type, traits_type>;
    class disposition;

private:
    struct is_iequal
    {
        bool operator()(value_type const & lhs, value_type const & rhs) const
        {
            return boost::algorithm::iequals(lhs, rhs);
        }
    };

    using multimap_type = acqua::container::sequenced_multimap<value_type, disposition, is_iequal>;
    using stringbuf_type = std::basic_stringbuf<char_type, traits_type, allocator_type>;
    using filebuf_type = std::basic_filebuf<char_type, traits_type>;

public:
    using size_type = typename multimap_type::size_type;
    using iterator = typename multimap_type::iterator;
    using const_iterator = typename multimap_type::const_iterator;
    using subpart_type = std::list<basic_message>;
    using subpart_iterator = typename subpart_type::iterator;
    using const_subpart_iterator = typename subpart_type::const_iterator;

public:
    virtual ~basic_message()
    {
    }

    /*!
     */
    void dump(std::ostream & os) const
    {
        for(auto const & a : header_) {
            os << a.first << ':' << ' ' << a.second;
            for(auto const & b : a.second)
                os << ' ' << b.first << '=' << '"' << b.second << '"';
            os << std::endl;
        }
        os << std::endl
           << str() << std::endl;
    }

    /*!
      メッセージ形式で、ペイロードバッファを作成する.
     */
    streambuf_type * set_payload()
    {
        body_.reset(new stringbuf_type());
        return body_.get();
    }

    /*!
      ファイル形式で、ペイロードバッファを作成する.

      ファイルが存在しない場合は新規作成する。
      ファイルが存在する場合 overwrite が false のときは追記モード、true のときは書き換えモードになる。
      どちらの場合でも、ファイルのオープンに失敗した場合は std::system_error を投げる
     */
    streambuf_type * set_payload(std::string const & filename, bool overwrite = false)
    {
        auto * fbuf = new filebuf_type();
        body_.reset(fbuf);
        if (overwrite) {
            fbuf->open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
            if (!fbuf->is_open()) {
                body_.release();
                acqua::exception::throw_error(std::make_error_code(std::errc::no_such_file_or_directory), filename);
            }
            fbuf->close();
        }
        fbuf->open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::app);
        if (!fbuf->is_open()) {
            body_.release();
            acqua::exception::throw_error(std::make_error_code(std::errc::permission_denied), filename);
        }
        return fbuf;
    }

    /*!
      set_payload() で作成されたペイロードバッファを返す.

      ペイロードバッファが作成されていないと nullptr を返す。
     */
    streambuf_type * get_payload() { return body_.get(); }

    /*!
      set_payload() で作成されたペイロードバッファを返す.

      ペイロードバッファが作成されていないと、メッセージ形式のペイロードバッファが作成される。
     */
    operator streambuf_type *() { return body_ ? get_payload() : set_payload(); }

    /*!
      本文を返す.

      バッファから文字列に全文コピーするため、かなり遅い
     */
    value_type str() const
    {
        if (auto * sbuf = dynamic_cast<stringbuf_type *>(body_.get())) {
            return sbuf->str();
        } else if (auto * fbuf = dynamic_cast<filebuf_type *>(body_.get())) {
            stringbuf_type sbuf;
            fbuf->pubseekpos(0, std::ios_base::in);
            std::basic_ostream<char_type, traits_type>(&sbuf) << fbuf;
            return sbuf.str();
        } else {
            return value_type();
        }
    }

    /*!
      filename に保存する.
     */
    bool save_as(std::string const & filename)
    {
        std::ofstream ofs(filename);
        if (body_) {
            body_->pubseekpos(0, std::ios_base::in);
            ofs << &*body_;
            return ofs.good();
        } else {
            return false;
        }
    }

    bool empty() const { return header_.empty(); }
    size_type size() const { return header_.size(); }
    void clear() { header_.clear(); }
    iterator begin() { return header_.begin(); }
    const_iterator begin() const { return header_.begin(); }
    iterator end() { return header_.end(); }
    const_iterator end() const { return header_.end(); }
    iterator find(value_type const & name) { return header_.find(name); }
    const_iterator find(value_type const & name) const { return header_.find(name); }
    iterator erase(const_iterator it) { return header_.erase(it); }
    iterator erase(const_iterator beg, const_iterator end) { return header_.erase(beg, end); }
    disposition & operator[](value_type const & name) { return header_[name]; }

    bool has_subpart() const
    {
        return !subpart_.empty();
    }

    basic_message & add_subpart()
    {
        subpart_.emplace_back();
        return subpart_.back();
    }
    static subpart_iterator begin(basic_message & mes) { return mes.subpart_.begin(); }
    static const_subpart_iterator begin(basic_message const & mes) { return mes.subpart_.begin(); }
    static subpart_iterator end(basic_message & mes) { return mes.subpart_.end(); }
    static const_subpart_iterator end(basic_message const & mes) { return mes.subpart_.end(); }

    using recursive_iterator = acqua::container::preordered_recursive_iterator<
        basic_message, subpart_iterator, &basic_message::begin, &basic_message::end>;

    using const_recursive_iterator = acqua::container::preordered_recursive_iterator<
        basic_message const, const_subpart_iterator, &basic_message::begin, &basic_message::end>;

    recursive_iterator recbegin() { return recursive_iterator(*this); }
    const_recursive_iterator recbegin() const { return const_recursive_iterator(*this); }
    recursive_iterator recend() { return recursive_iterator(); }
    const_recursive_iterator recend() const { return const_recursive_iterator(); }

private:
    std::unique_ptr<streambuf_type> body_;
    multimap_type header_;
    subpart_type subpart_;
};


template <typename String>
class basic_message<String>::disposition
    : private boost::totally_ordered<disposition, value_type>
{
    using map_type = acqua::container::sequenced_map<value_type, value_type, is_iequal>;

public:
    using size_type = typename map_type::size_type;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

public:
    disposition & operator=(char_type data) { assign(data); return *this; }
    disposition & operator=(value_type const & data) { assign(data); return *this; }
    disposition & operator+=(char_type data) { append(data); return *this; }
    disposition & operator+=(value_type const & data) { append(data); return *this; }
    operator value_type &() { return data_; }
    operator value_type const &() const { return data_; }
    friend bool operator==(disposition const & lhs, value_type const & rhs) { return lhs.data_ == rhs; }
    friend bool operator<(disposition const & lhs, value_type const & rhs) { return lhs.data_ < rhs; }
    friend ostream_type & operator<<(ostream_type & os, disposition const & rhs) { return os << rhs.str(); }
    value_type & str() { return data_; }
    value_type const & str() const { return data_; }
    void assign(char_type ch) { data_.assign(ch); }
    void assign(value_type const & str) { data_.assign(str); }
    void assign(char_type const * str, std::size_t size) { data_.assign(str, size); }
    template <typename It> void assign(It beg, It end) { data_.assign(beg, end); }
    void append(char_type ch) { data_.append(ch); }
    void append(value_type const & str) { data_.append(str); }
    void append(char_type const * str, std::size_t size) { data_.append(str, size); }
    template <typename It> void append(It beg, It end) { data_.append(beg, end); }
    bool empty() const { return param_.empty(); }
    size_type size() const { return param_.size(); }
    void clear() { param_.clear(); }
    iterator begin() { return param_.begin(); }
    const_iterator begin() const { return param_.begin(); }
    iterator end() { return param_.end(); }
    const_iterator end() const { return param_.end(); }
    iterator find(value_type const & name) { return param_.find(name); }
    const_iterator find(value_type const & name) const { return param_.find(name); }
    value_type & operator[](value_type const & name) { return param_[name]; }

private:
    value_type data_;
    map_type param_;
};

} }
