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
#include <acqua/text/adapted/json_adaptor.hpp>
#include <acqua/text/impl/json_feed_parser_state.hpp>

namespace acqua { namespace text {

template <typename CharT, typename Json, typename Adapt = acqua::text::adapted::json_adaptor<Json> >
class json_feed_parser
{
    using state_type = acqua::text::impl::json_feed_parser_state<CharT, Json, Adapt>;

public:
    json_feed_parser(Json & json)
        : state_(new state_type(error_, json)) {}

    /*!
      パースが終了したか.
     */
    bool is_terminated() const
    {
        return (bool)error_ || state_->is_terminated();
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
    json_feed_parser & parse(CharT ch)
    {
        while(!state_->do_parse_1(ch))
            ;
        return *this;
    }

    /*!
      is がEOFになるか、パースが正常/異常終了するまで、is から文字を取得し続ける.
    */
    friend std::basic_istream<CharT> & operator>>(std::basic_istream<CharT> & is, json_feed_parser<CharT, Json, Adapt> & rhs)
    {
        CharT ch;
        while(!rhs.is_terminated() && is.get(ch))
            rhs.parse(ch);
        return is;
    }

private:
    boost::system::error_code error_;
    std::unique_ptr<state_type> state_;
};

} }

#include <acqua/text/impl/json_feed_parser_state.ipp>
