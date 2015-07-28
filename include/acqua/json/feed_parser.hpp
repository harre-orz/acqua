/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <memory>
#include <boost/system/error_code.hpp>
#include <acqua/json/adapted_proto.hpp>

namespace acqua { namespace json {

template <
    typename CharT,
    typename Json,
    typename Adapt = adapted<Json>
    >
class feed_parser
{
    class impl;

public:
    feed_parser(Json & json)
        : impl_(new impl(error_, json)) {}

    /*!
      パースが終了したか.
     */
    bool is_terminated() const
    {
        return (bool)error_ || impl_->is_terminated();
    }

    /*!
      パースが継続できないとき、error に値が入る
    */
    boost::system::error_code const & get_error_code() const
    {
        return error_;
    }

    /*!
      1文字パースする.
    */
    feed_parser & parse(CharT ch)
    {
        while(!impl_->do_parse_1(ch))
            ;
        return *this;
    }

    /*!
      is がEOFになるか、パースが正常/異常終了するまで、is から文字を取得し続ける.
    */
    friend std::basic_istream<CharT> & operator>>(std::basic_istream<CharT> & is, feed_parser<CharT, Json, Adapt> & rhs)
    {
        CharT ch;
        while(!rhs.is_terminated() && is.get(ch))
            rhs.parse(ch);
        return is;
    }

private:
    boost::system::error_code error_;
    std::unique_ptr<impl> impl_;
};

} }

#include <acqua/json/impl/feed_parser_impl.ipp>
