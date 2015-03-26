 /*!
  acqua library

  Copyright (c) 2015 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#pragma once

#include <iostream>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <acqua/asio/read_until.hpp>


namespace acqua { namespace website { namespace detail {

template <typename Result>
class client_socket_base
    : public std::ostream
{
public:
    using endpoint_type = boost::asio::ip::tcp::endpoint;
    using buffer_type = typename Result::buffer_type;
    using result_type = typename Result::result;

public:
    client_socket_base()
        : std::ostream(&buffer_) {}

    virtual ~client_socket_base() {}
    virtual void cancel() = 0;

    endpoint_type const & endpoint() const noexcept
    {
        return endpoint_;
    }

    bool is_keep_alive() const noexcept
    {
        return is_keep_alive_;
    }

    boost::shared_ptr<Result> start()
    {
        decltype(result_) expected;
        decltype(result_) desired(new Result());
        if (!boost::atomic_compare_exchange(&result_, &expected, desired)) {
            return nullptr;
        }

        async_write();
        return boost::static_pointer_cast<Result>(result_);
    }

    template <typename Handler>
    void async_start(Handler handler)
    {
        decltype(result_) expected;
        decltype(result_) desired(new result_type(handler));
        if (!boost::atomic_compare_exchange(&result_, &expected, desired)) {
            handler(boost::system::error_code(), *desired);
            return;
        }

        async_write();
    }

protected:
    virtual void async_write() = 0;

    void callback(boost::system::error_code const & error, boost::asio::io_service & io_service)
    {
        if (auto result = boost::atomic_exchange(&result_, decltype(result_)())) {
            io_service.post([error, result]() {
                    result->handler_(error, *result);
                });
        }
    }

    void buffer_copy(std::size_t size)
    {
        std::istream is(&buffer_);
        std::ostream os(&result_->buffer_);
        std::copy_n(std::istreambuf_iterator<char>(is), size, std::ostreambuf_iterator<char>(os));
    }

    typename result_type::header_type & get_header()
    {
        return result_->header_;
    }

protected:
    endpoint_type endpoint_;
    boost::shared_ptr<result_type> result_;
    buffer_type buffer_;
    bool is_keep_alive_ = false;
};



template <typename Client, typename Result, typename Socket, typename Timer>
class basic_client_socket
    : public client_socket_base<Result>
    , public std::enable_shared_from_this< basic_client_socket<Client, Result, Socket, Timer> >
{
    using base_type = client_socket_base<Result>;

public:
    using socket_type = Socket;
    using timer_type = Timer;
    using resolver_type = boost::asio::ip::tcp::resolver;
    using endpoint_type = typename base_type::endpoint_type;
    using buffer_type = typename base_type::buffer_type;
    using result_type = typename base_type::result_type;

    explicit basic_client_socket(Client * client, boost::asio::io_service & io_service)
        : client_(client), socket_(io_service), timer_(io_service), is_ready_(false), retry_(1) {}

    template <typename Clock, typename Duration>
    void timeout(std::chrono::time_point<Clock, Duration> const & time_point)
    {
        timer_.expires_at(time_point);
        timer_.async_wait(std::bind(&basic_client_socket::on_wait, this->shared_from_this(), std::placeholders::_1));
    }

    template <typename Rep, typename Period>
    void timeout(std::chrono::duration<Rep, Period> const & duration)
    {
        timer_.expires_from_now(duration);
        timer_.async_wait(std::bind(&basic_client_socket::on_wait, this->shared_from_this(), std::placeholders::_1));
    }

    void cancel()
    {
        socket_.cancel();
    }

    void async_connect(endpoint_type const & endpoint)
    {
        base_type::endpoint_ = endpoint;

        if (auto socket = client_->reuse(this)) {
            socket->async_reuse(this);
        } else {
            socket_.async_connect(
                base_type::endpoint_,
                std::bind(
                    &basic_client_socket::on_connect1,
                    this->shared_from_this(),
                    std::placeholders::_1
                )
            );
        }
    }

    void async_connect(boost::system::error_code const & error, typename resolver_type::iterator it)
    {
        if (it != typename resolver_type::iterator()) {
            base_type::endpoint_ = it->endpoint();
            if (auto socket = client_->reuse(this)) {
                socket->async_reuse(this);
            } else {
                socket_.async_connect(
                    base_type::endpoint_,
                    std::bind(
                        &basic_client_socket::on_connect2,
                        this->shared_from_this(),
                        std::placeholders::_1, it
                    )
                );
            }
        } else {
            on_error(error, "async_connect");
        }
    }

    void async_write()
    {
        if (base_type::result_) {
            if (is_ready_.exchange(false) == true) {
                boost::asio::async_write(
                    socket_,
                    boost::asio::buffer(
                        boost::asio::buffer_cast<char const *>(base_type::buffer_.data()),
                        base_type::buffer_.size()),
                    std::bind(
                        &basic_client_socket::on_write,
                        this->shared_from_this(),
                        std::placeholders::_1
                    )
                );
            }
        }
    }

private:
    void async_reconnect()
    {
        socket_.async_connect(
            base_type::endpoint_,
            std::bind(
                &basic_client_socket::on_connect1,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void on_connect1(boost::system::error_code const & error)
    {
        if (!error) {
            is_ready_ = true;
            async_write();
        } else {
            on_error(error, "on_connect1");
        }
    }

    void on_connect2(boost::system::error_code const & error, typename resolver_type::iterator it)
    {
        if (!error) {
            is_ready_ = true;
            async_write();
        } else {
            boost::system::error_code ec;
            socket_.close(ec);
            if (ec) on_error(ec, "on_connect2");
            async_connect(error, ++it);
        }
    }

    void on_write(boost::system::error_code const & error)
    {
        if (!error) {
            async_read();
        } else {
            on_error(error, "on_write");
        }
    }

    void async_read()
    {
        socket_.async_read_some(
            boost::asio::buffer(&buffer1_, 1),
            std::bind(
                &basic_client_socket::on_read_1,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void on_read_1(boost::system::error_code const & error)
    {
        if (!error) {
            base_type::buffer_.consume(base_type::buffer_.size());
            boost::asio::async_read_until(
                socket_, base_type::buffer_, "\r\n",
                std::bind(
                    &basic_client_socket::on_read_line,
                    this->shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2
                )
            );
        } else if (--retry_ > 0) {
            socket_.close();
            async_reconnect();
        } else {
            on_error(error, "on_read_1");
        }
    }

    void on_read_line(boost::system::error_code const & error, std::size_t size)
    {
        if (!error) {
            base_type::buffer_.consume(size);
            boost::asio::async_read_until(
                socket_, base_type::buffer_, "\r\n\r\n",
                std::bind(
                    &basic_client_socket::on_read_header,
                    this->shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2
                )
            );
        } else {
            on_error(error, "on_read_line");
        }
    }

    void on_read_header(boost::system::error_code const & error, std::size_t size)
    {
        if (!error) {
            auto & header = base_type::get_header();
            auto beg = boost::asio::buffer_cast<char const *>(base_type::buffer_.data());
            auto end = beg + size;

            namespace qi = boost::spirit::qi;
            qi::parse(beg, end, (
                          *(qi::char_ - ':') >> ':' >> qi::omit[ *qi::space ] >> *(qi::char_ - "\r\n")
                      ) % "\r\n" >> "\r\n", header);
            base_type::buffer_.consume(size);

            auto it = header.find("Connection");
            if (it != header.end()) {
                base_type::is_keep_alive_ = boost::iequals(it->second, "keep-alive");
            }

            it = header.find("Transfer-Encoding");
            if (it != header.end() && boost::iequals(it->second, "chunked")) {
                boost::asio::async_read_until(
                    socket_, base_type::buffer_, "\r\n",
                    std::bind(
                        &basic_client_socket::on_read_chunked_size,
                        this->shared_from_this(),
                        std::placeholders::_1, std::placeholders::_2
                    )
                );
            } else {
                it = header.find("Content-Length");
                if (it != header.end()) {
                    size = std::atol(it->second.c_str());
                    boost::asio::async_read_until(
                        socket_, base_type::buffer_, size,
                        std::bind(
                            &basic_client_socket::on_read_sized_content,
                            this->shared_from_this(),
                            std::placeholders::_1, std::placeholders::_2
                        )
                    );
                } else {
                    size = -1;
                    boost::asio::async_read(
                        socket_, base_type::buffer_,
                        std::bind(
                            &basic_client_socket::on_read_sized_content,
                            this->shared_from_this(),
                            std::placeholders::_1, std::placeholders::_2
                        )
                    );
                }
            }
        } else {
            on_error(error, "on_read_header");
        }
    }

    void on_read_sized_content(boost::system::error_code const & error, std::size_t size)
    {
        if (!error) {
            timer_.cancel();
            base_type::buffer_copy(size);
            base_type::callback(error, socket_.get_io_service());
        } else {
            on_error(error, "on_read_sized_content");
        }
    }

    void on_read_chunked_size(boost::system::error_code const & error, std::size_t size)
    {
        if (!error && size > 2) {
            auto data = boost::asio::buffer_cast<char const *>(base_type::buffer_.data());
            const_cast<char *>(data)[size-2] = '\0';
            std::size_t chunk_size = std::strtol(data, nullptr, 16);

            base_type::buffer_.consume(size);
            if (chunk_size != 0) {
                boost::asio::async_read_until(
                    socket_, base_type::buffer_, chunk_size+2,
                    std::bind(
                        &basic_client_socket::on_read_chunked_content,
                        this->shared_from_this(),
                        std::placeholders::_1, std::placeholders::_2
                    )
                );
            } else {
                timer_.cancel();
                base_type::callback(error, socket_.get_io_service());
            }
        } else {
            on_error(error, "on_read_chunked_size");
        }
    }

    void on_read_chunked_content(boost::system::error_code const & error, std::size_t size)
    {
        if (!error) {
            base_type::buffer_copy(size);
            boost::asio::async_read_until(
                socket_, base_type::buffer_, "\r\n",
                std::bind(
                    &basic_client_socket::on_read_chunked_size,
                    this->shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2
                )
            );
        } else {
            on_error(error, "on_read_chunked_content");
        }
    }

    void on_error(boost::system::error_code const & error, char const * location)
    {
        timer_.cancel();
        std::cout << "on_error " << this << location << std::endl;
        base_type::callback(error, socket_.get_io_service());
    }

    void on_wait(boost::system::error_code const & error)
    {
        if (!error) {
            socket_.cancel();
        }
    }

public:
    void async_keep_alive()
    {
        socket_.async_receive(
            boost::asio::buffer(&buffer1_, 1),
            std::bind(
                &basic_client_socket::on_keep_alive,
                this
            )
        );
    }

private:
    void on_keep_alive()
    {
        if (reuse_) {
            reuse_->socket_ = std::move(socket_);
            reuse_->on_connect1(boost::system::error_code());
        } else {
            client_->erase(this);
        }

        delete this;
    }

    void async_reuse(basic_client_socket * socket)
    {
        reuse_ = socket->shared_from_this();
        socket_.cancel();
    }

private:
    Client * client_;
    socket_type socket_;
    timer_type timer_;
    std::atomic<bool> is_ready_;
    char buffer1_;
    std::shared_ptr<basic_client_socket> reuse_;
    int retry_;
};


} } }
