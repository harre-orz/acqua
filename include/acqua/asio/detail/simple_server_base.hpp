/*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <memory>
#include <limits>
#include <atomic>
#include <type_traits>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace asio { namespace detail {

/*!
  Stream Protocol の接続数を管理するクラス

  \tparam Enabler Connector クラスは、std::enable_shared_from_this<Connector> を継承をチェックするSFINAE
 */
template <
    typename Derived,
    typename Connector,
    typename Protocol,
    typename Tag = void,
    typename Enabler = void
    >
class simple_server_base
{
protected:
    using protocol_type = Protocol;
    using acceptor_type = typename Protocol::acceptor;
    using endpoint_type = typename Protocol::endpoint;

    ~simple_server_base() noexcept;

    acceptor_type & acceptor();

public:
    explicit simple_server_base(boost::asio::io_service & io_service, bool volatile & marked_alive, std::atomic<std::size_t> & count);

    void start();

    void stop();
};


/*!
 */
template <
    typename Derived,
    typename Connector,
    typename Protocol,
    typename Tag
    >
class simple_server_base<
    Derived, Connector, Protocol, Tag,
    typename std::enable_if<std::is_base_of<std::enable_shared_from_this<Connector>, Connector>::value>::type
    > : boost::noncopyable
{
protected:
    using protocol_type = Protocol;
    using acceptor_type = typename Protocol::acceptor;
    using endpoint_type = typename Protocol::endpoint;

    ~simple_server_base() noexcept
    {
        marked_alive_ = false;
    }

    acceptor_type & acceptor()
    {
        return acceptor_;
    }

public:
    explicit simple_server_base(boost::asio::io_service & io_service, bool volatile & marked_alive, std::atomic<std::size_t> & count)
        : acceptor_(io_service)
        , count_(count)
        , is_running_(false)
        , is_waiting_(false)
        , marked_alive_(marked_alive)
    {
        marked_alive = true;
    }

    //! 非同期の接続待ち状態を開始する.
    void start()
    {
        if (acceptor_.is_open()) {
            if (is_running_.exchange(true) == false) {
                is_waiting_ = false;
                async_accept();
            }
        }
    }

    //! 非同期の接続待ち状態を解除する.
    void stop()
    {
        is_running_ = false;
        acceptor_.cancel();
    }

private:
    void on_disconnect(Connector * conn, bool volatile & marked_alive)
    {
        delete conn;
        if (marked_alive) {
            count_--;
            if (is_waiting_.exchange(false) == true) {
                async_accept();
            }
        }
    }

    void async_accept()
    {
        // async_accept と on_disconnect が並列に呼び出されると、
        // レースコンディションになるので、十分に注意して実装すること。
        if (count_ < max_count()) {
            ++count_;
            // count_ はここでしか加算していないので、
            // count_ < max_count() の条件を満たせなくなることはない

            std::shared_ptr<Connector> conn(
                static_cast<Derived *>(this)->construct(acceptor_.get_io_service()),
                std::bind(&simple_server_base::on_disconnect, this, std::placeholders::_1, std::ref(marked_alive_))
            );
            acceptor_.async_accept(
                lowest_layer_socket(conn->socket()),
                std::bind(&simple_server_base::on_accept, this, std::placeholders::_1, conn)
            );
        } else {
            // 発生しにくいが is_waiting_ のフラグが設定する直前に
            // on_disconnect のフラグ処理が行われると、accept しない状態になる。
            // たとえ accept しない状態になったとしても、十分に max_count を設定していれば、
            // 次の connector の破棄のタイミングで復活するから大丈夫。
            is_waiting_ = true;
        }
    }

    void on_accept(boost::system::error_code const & error, std::shared_ptr<Connector> & conn)
    {
        if (!error) {
            async_accept();
            conn->start();
        } else if (is_running_.exchange(false) == true) {
            acqua::exception::throw_error(error, "accept");
        }
    }

    using socket_type = typename Protocol::socket;

    // for basic_socket
    static socket_type & lowest_layer_socket(socket_type & socket)
    {
        return socket;
    }

    // for ssl_socket
    template <typename Socket>
    static typename Socket::lowest_layer_type & lowest_layer_socket(Socket & socket, typename Socket::lowest_layer_type * = nullptr)
    {
        return socket.lowest_layer();
    }

    //! 最大接続数を返す.
    std::size_t max_count() const noexcept
    {
        return static_cast<Derived const *>(this)->max_count();
    }

private:
    acceptor_type acceptor_;
    std::atomic<std::size_t> & count_;
    std::atomic<bool> is_running_;
    std::atomic<bool> is_waiting_;
    bool volatile & marked_alive_;
};

} } }
