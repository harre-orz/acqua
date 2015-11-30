#pragma once

#include <acqua/iostreams/json_parser.hpp>
#include <acqua/iostreams/detail/json_parser_impl.hpp>

namespace acqua { namespace iostreams {

template <typename Json, typename Adapt, typename CharT>
struct json_parser<Json, Adapt, CharT>::impl
    : detail::json_parser_impl<Json, Adapt, CharT>
{
    impl(boost::system::error_code & error, Json & json)
        : detail::json_parser_impl<Json, Adapt, CharT>(error, json) {}
};


template <typename Json, typename Adapt, typename CharT>
inline json_parser<Json, Adapt, CharT>::json_parser(Json & json)
    : error_(), impl_(new impl(error_, json)) {}


template <typename Json, typename Adapt, typename CharT>
inline std::streamsize json_parser<Json, Adapt, CharT>::write(char_type const * s, std::streamsize n)
{
    if (!impl_->is_in_progress)
        return EOF;

    std::streamsize i;
    for(i = 0; impl_->is_in_progress && i < n; ++i, ++s) {
        while(impl_->is_in_progress && !impl_->parse_1(*s));
    }
    return i;
}

} }
