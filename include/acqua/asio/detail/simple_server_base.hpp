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
#include <boost/asio/io_service.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/ip/v6_only.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/make_fused.hpp>
#include <acqua/asio/detail/socket_category.hpp>

namespace acqua { namespace asio { namespace detail {

/*!
  サーバソケットの接続数を管理するクラス.
  \tparam Enabler Connector クラスは、std::enable_shared_from_this<Connector> を継承していること
*/
template <
    typename Derived,
    typename Connector,
    typename Protocol,
    typename Tag = unspecified_tag
    >
class simple_server_base
{
protected:
    using base_type = simple_server_base;

public:
    using protocol_type = Protocol;
    using acceptor_type = typename Protocol::acceptor;
    using endpoint_type = typename Protocol::endpoint;
    using size_type = std::size_t;

protected:
    using atomic_size_type = std::atomic<size_type>;
    using atomic_bool_type = std::atomic<bool>;

    ~simple_server_base() = default;

private:
    struct placeholder
    {
        virtual std::shared_ptr<Connector> construct(simple_server_base * this_, boost::asio::io_service & io_service) = 0;
        virtual ~placeholder() = default;
    };

    template <typename... Args>
    struct holder
        : placeholder
    {
        explicit holder(Args... args)
            : args_(args...)
        {
        }

        virtual std::shared_ptr<Connector> construct(simple_server_base * this_, boost::asio::io_service & io_service)
        {
            return boost::fusion::make_fused(&simple_server_base::construct_impl<Args...>)(
                boost::fusion::push_front(boost::fusion::push_front(args_, std::ref(io_service)), this_));
        }

        boost::fusion::vector<Args...> args_;
    };

public:
    template <typename... Args>
    explicit simple_server_base(boost::asio::io_service & io_service, Args... args)
        : acceptor_(io_service)
        , holder_(std::make_unique< holder<Args...> >(args...))
        , is_running_(false)
        , is_waiting_(false)
    {
    }

    void listen(endpoint_type const & endpoint, boost::system::error_code & ec, bool reuse_addr = true)
    {
        protocol_type proto = endpoint.protocol();
        acceptor_.open(proto, ec);
        if (ec) return;

        if (reuse_addr) {
            set_reuseaddr(proto, ec);
            if (ec) return;
        }

        set_v6only(Tag(), proto, ec);
        if (ec) return;

        static_cast<Derived *>(this)->set_option(acceptor_, proto, ec);
        if (ec) return;

        acceptor_.bind(endpoint, ec);
        if (ec) return;

        acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    }

    void start(boost::system::error_code & ec)
    {
        if (acceptor_.is_open()) {
            if (is_running_.exchange(true) == false) {
                async_accept();
            } else {
                ec = make_error_code(boost::asio::error::already_started);
            }
        } else {
            ec = make_error_code(boost::asio::error::not_socket);
        }
    }

    void cancel(boost::system::error_code & ec)
    {
        is_running_ = false;
        acceptor_.cancel(ec);
    }

    void close(boost::system::error_code & ec)
    {
        is_running_ = false;
        acceptor_.close(ec);
    }

protected:
    acceptor_type & get_acceptor()
    {
        return acceptor_;
    }

private:
    std::shared_ptr<Connector> construct(boost::asio::io_service & io_service)
    {
        return holder_->construct(this, io_service);
    }

    template <typename... Args>
    static std::shared_ptr<Connector> construct_impl(simple_server_base * this_, boost::asio::io_service & io_service, Args... args)
    {
        return static_cast<Derived *>(this_)->construct(
            std::bind(&simple_server_base::on_disconnect, this_, std::placeholders::_1),
            io_service, args...);
    }

    //! true のときは async_accept() を行う
    bool incr()
    {
        if (is_waiting_)
            return false;
        if (++static_cast<Derived *>(this)->use_count_ >= static_cast<Derived *>(this)->max_count_ && is_waiting_.exchange(true) == true) {
            static_cast<Derived *>(this)->use_count_--;
            return false;
        }
        return true;
    }

    //! true のときは async_accept() を行う
    bool decl()
    {
        if (acceptor_.get_io_service().stopped())
            return false;
        return --static_cast<Derived *>(this)->use_count_ < static_cast<Derived *>(this)->max_count_ && is_waiting_.exchange(false) == true;
    }

    void on_disconnect(Connector * conn)
    {
        static_assert(noexcept(static_cast<Derived *>(this)->destruct(conn)), "must be noexcept");
        static_cast<Derived *>(this)->destruct(conn);
        try {
            if (decl()) {
                async_accept();
            }
        } catch(...) {
            is_running_ = false;
            is_waiting_ = false;
        }
    }

    void on_accept(boost::system::error_code const & error, std::shared_ptr<Connector> conn)
    {
        if (error) {
            is_running_ = false;
            is_waiting_ = false;
            boost::asio::detail::throw_error(error, "on_accept");
        } else if (is_running_) {
            async_accept();
            static_cast<Derived *>(this)->start(conn);
        }
    }

    void async_accept()
    {
        if (incr()) {
            std::shared_ptr<Connector> conn(holder_->construct(this, acceptor_.get_io_service()));
            acceptor_.async_accept(
                static_cast<Derived *>(this)->socket(conn),
                std::bind(&simple_server_base::on_accept, this, std::placeholders::_1, conn));
        }
    }

    template <typename Protocol_>
    void set_reuseaddr(Protocol_ const &, boost::system::error_code & ec)
    {
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    }

    template <typename Tag_, typename Protocol_>
    void set_v6only(Tag_, Protocol_ const &, boost::system::error_code &)
    {
    }

    void set_v6only(unspecified_tag, boost::asio::ip::tcp const & proto, boost::system::error_code & ec)
    {
        if (proto == boost::asio::ip::tcp::v6()) {
            acceptor_.set_option(boost::asio::ip::v6_only(true), ec);
        }
    }

    void set_v6only(internet_v6_tag, protocol_type const &, boost::system::error_code & ec)
    {
        acceptor_.set_option(boost::asio::ip::v6_only(true), ec);
    }

private:
    acceptor_type acceptor_;
    std::unique_ptr<placeholder> holder_;
    atomic_bool_type is_running_;
    atomic_bool_type is_waiting_;
};

} } }
