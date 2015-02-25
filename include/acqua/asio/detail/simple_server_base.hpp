#pragma once

#include <memory>
#include <limits>
#include <atomic>
#include <type_traits>

#include <boost/utility.hpp>
#include <boost/asio.hpp>

namespace acqua { namespace asio { namespace detail {

/*!
 */
template <
    typename Derived,
    typename Connector,
    typename Protocol,
    typename Tag = void,
    typename Enable = void
    >
class simple_server_base;


/*!
 */
template <typename Derived, typename Connector, typename Protocol, typename Tag>
class simple_server_base<
    Derived, Connector, Protocol, Tag,
    typename std::enable_if<std::is_base_of<std::enable_shared_from_this<Connector>, Connector>::value>::type  // SFINAE
    > : boost::noncopyable
{
protected:
    typedef std::size_t size_type;
    typedef Protocol protocol_type;
    typedef typename Protocol::socket socket_type;
    typedef typename Protocol::acceptor acceptor_type;
    typedef typename Protocol::endpoint endpoint_type;

    ~simple_server_base()
    {
        marked_alive_ = false;
    }

    acceptor_type & acceptor()
    {
        return acceptor_;
    }

public:
    explicit simple_server_base(boost::asio::io_service & io_service, bool volatile & marked_alive, size_type max_count, std::atomic<size_type> & count)
        : acceptor_(io_service)
        , max_count_(max_count)
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
        if (acceptor_.is_open() && is_running_.exchange(true) == false) {
            is_waiting_ = false;
            async_accept();
        }
    }

    //! 非同期の接続待ち状態を解除する.
    void stop()
    {
        acceptor_.cancel();
        is_running_ = false;
    }

    //! 現在の接続数を返す.
    size_type use_count() noexcept
    {
        return count_;
    }

    //! 最大接続数を返す.
    size_type max_count() noexcept
    {
        return max_count_;
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
        if (count_ < max_count_) {
            ++count_;
            // count_ はここでしか加算していないので、
            // count_ < max_count_ の条件を満たせなくなることはない

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
        } else {
            is_running_ = false;
            if (error != boost::system::error_code(boost::asio::error::operation_aborted, boost::asio::error::get_system_category())) {
                throw boost::system::system_error(error);
            }
        }
    }

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

protected:
    acceptor_type acceptor_;
    size_type max_count_;
    std::atomic<size_type> & count_;
    std::atomic<bool> is_running_;
    std::atomic<bool> is_waiting_;
    bool volatile & marked_alive_;
};

} } }
