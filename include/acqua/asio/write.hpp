#pragma once

#include <boost/asio/write.hpp>
#include <boost/asio/basic_streambuf.hpp>
#include <boost/asio/completion_condition.hpp>

namespace boost { namespace asio {

/*!
  streambuf を使い、size バイトまでを送信し、streambuf を消費する.
 */
template <typename SyncWriteStream, typename Allocator>
std::size_t write(SyncWriteStream & s, basic_streambuf<Allocator> & b, std::size_t size)
{
    return write(s, b, transfer_exactly(size));
}

/*!
  streambuf を使い、size バイトまでを送信し、streambuf を消費する.
 */
template <typename SyncWriteStream, typename Allocator>
std::size_t write(SyncWriteStream & s, basic_streambuf<Allocator> & b, std::size_t size, boost::system::error_code & ec)
{
    return write(s, b, transfer_exactly(size), ec);
}

/*!
  streambuf を使い、size バイトまでを非同期に送信し、streambuf を消費する.
 */
template <typename AsyncWriteStream, typename Allocator, typename WriteHandler>
void async_write(AsyncWriteStream & s, basic_streambuf<Allocator> & b, std::size_t size, WriteHandler handler)
{
    async_write(s, b, transfer_exactly(size), handler);
}

} }
