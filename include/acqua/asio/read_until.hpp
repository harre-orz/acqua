/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <boost/asio/read_until.hpp>

namespace acqua { namespace asio {

namespace detail {

class match_buffer_size
{
    std::size_t size_;

public:
    explicit match_buffer_size(std::size_t size)
        : size_(size)
    {
    }

    template <typename Iterator>
    std::pair<Iterator, bool> operator()(Iterator it, Iterator end)
    {
        std::size_t d = std::distance(it, end);
        if (size_ <= d) {
            std::advance(it, size_);
            return std::make_pair(it, true);
        } else {
            size_ -= d;
            return std::make_pair(end, false);
        }
    }
};

} } }

namespace boost { namespace asio {

template <>
struct is_match_condition<acqua::asio::detail::match_buffer_size>
    : public boost::true_type
{
};

/*!
  streambuf を使い、size バイトまで受信する (特殊化)
 */
template<typename AsyncReadStream, typename Allocator, typename ReadHandler>
auto async_read_until(AsyncReadStream & s, basic_streambuf< Allocator > & b, std::size_t size, ReadHandler handler) -> void
{
    async_read_until(s, b, acqua::asio::detail::match_buffer_size(size), handler);
}

} }
