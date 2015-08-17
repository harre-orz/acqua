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
#include <boost/asio/ssl.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <acqua/asio/read_until.hpp>
#include <acqua/webclient/uri.hpp>
#include <acqua/webclient/wget.hpp>
#include <acqua/webclient/wpost.hpp>
#include <acqua/webclient/detail/client_socket_base.hpp>

namespace acqua { namespace webclient { namespace detail {

template <typename Client, typename Result, typename Socket, typename Timer>
class client_socket
    : public client_socket_base<Result>
    , public std::enable_shared_from_this< client_socket<Client, Result, Socket, Timer> >
{
    using base_type = client_socket_base<Result>;

public:
    using socket_type = Socket;
    using timer_type = Timer;
    using resolver_type = boost::asio::ip::tcp::resolver;
    using endpoint_type = typename base_type::endpoint_type;
    using buffer_type = typename base_type::buffer_type;
    using result_type = typename base_type::result_type;

    client_socket(Client * client, boost::asio::io_service & io_service, int retry = 3)
        : client_(client), socket_(io_service), timer_(io_service), retry_(retry), is_ready_(false) {}

    client_socket(Client * client, boost::asio::io_service & io_service, boost::asio::ssl::context & ctx, int retry = 3)
        : client_(client), socket_(io_service, ctx), timer_(io_service), retry_(retry), is_ready_(false) {}

    void set_verify_none()
    {
        socket_.set_verify_mode(boost::asio::ssl::verify_none);
    }

    void set_verify_peer()
    {
        socket_.set_verify_mode(boost::asio::ssl::verify_peer);
        socket_.set_verify_callback(
            std::bind(
                &client_socket::verify_certificate,
                this->shared_from_this(),
                std::placeholders::_1, std::placeholders::_2
            )
        );
    }

    template <typename Clock, typename Duration>
    void set_timeout(std::chrono::time_point<Clock, Duration> const & time_point)
    {
        timer_.expires_at(time_point);
        timer_.async_wait(std::bind(&client_socket::on_wait, this->shared_from_this(), std::placeholders::_1));
    }

    template <typename Rep, typename Period>
    void set_timeout(std::chrono::duration<Rep, Period> const & duration)
    {
        timer_.expires_from_now(duration);
        timer_.async_wait(std::bind(&client_socket::on_wait, this->shared_from_this(), std::placeholders::_1));
    }

    void cancel()
    {
        socket_.lowest_layer().cancel();
    }

    void async_connect(endpoint_type const & endpoint)
    {
        base_type::endpoint_ = endpoint;

        if (auto socket = client_->reuse(this)) {
            socket->async_reuse(this);
        } else {
            socket_.lowest_layer().async_connect(
                base_type::endpoint_,
                std::bind(
                    &client_socket::on_connect1,
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
                lowest_layer_socket(socket_).async_connect(
                    base_type::endpoint_,
                    std::bind(
                        &client_socket::on_connect2,
                        this->shared_from_this(),
                        std::placeholders::_1, it
                    )
                );
            }
        } else {
            is_ready_ = true;
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
                        &client_socket::on_write,
                        this->shared_from_this(),
                        std::placeholders::_1
                    )
                );
            }
        }
    }

private:
    template <typename T>
    static typename T::lowest_layer_type & lowest_layer_socket(T & socket, typename T::lowest_layer_type * = nullptr)
    {
        return socket.lowest_layer();
    }

    void async_reconnect()
    {
        lowest_layer_socket(socket_).async_connect(
            base_type::endpoint_,
            std::bind(
                &client_socket::on_connect1,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void on_connect1(boost::system::error_code const & error)
    {
        is_ready_ = true;
        if (!error) {
            async_handshake(socket_);
        } else {
            on_error(error, "on_connect1");
        }
    }

    void on_connect2(boost::system::error_code const & error, typename resolver_type::iterator it)
    {
        if (!error) {
            is_ready_ = true;
            async_handshake(socket_);
        } else {
            boost::system::error_code ec;
            lowest_layer_socket(socket_).close(ec);
            if (ec) on_error(ec, "on_connect2");
            async_connect(error, ++it);
        }
    }

    template <typename Protocol>
    void async_handshake(boost::asio::basic_stream_socket<Protocol> &)
    {
        async_write();
    }

    template <typename T>
    void async_handshake(T &, typename T::lowest_layer_type * = nullptr)
    {
        socket_.async_handshake(
            boost::asio::ssl::stream_base::client,
            std::bind(
                &client_socket::on_handshake,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void on_handshake(boost::system::error_code const & error)
    {
        is_ready_ = true;
        if (!error) {
            async_write();
        } else {
            on_error(error, "on_handshake");
        }
    }

    void on_write(boost::system::error_code const & error)
    {
        if (!error) {
            async_read();
        } else if (retry_-- > 0) {
            lowest_layer_socket(socket_).close();
            async_reconnect();
        } else {
            on_error(error, "on_write");
        }
    }

    void async_read()
    {
        socket_.async_read_some(
            boost::asio::buffer(&buffer1_, 1),
            std::bind(
                &client_socket::on_read_1,
                this->shared_from_this(),
                std::placeholders::_1
            )
        );
    }

    void on_read_1(boost::system::error_code const & error)
    {
        if (!error) {
            std::ostream(&(base_type::temp_buffer())) << buffer1_;
            boost::asio::async_read_until(
                socket_, base_type::temp_buffer(), "\r\n",
                std::bind(
                    &client_socket::on_read_line,
                    this->shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2
                )
            );
        } else if (retry_-- > 0) {
            lowest_layer_socket(socket_).close();
            async_reconnect();
        } else {
            on_error(error, "on_read_1");
        }
    }

    void on_read_line(boost::system::error_code const & error, std::size_t size)
    {
        if (!error) {
            char const * data = boost::asio::buffer_cast<char const *>(base_type::temp_buffer().data());
            namespace qi = boost::spirit::qi;
            if (!qi::parse(data, data + base_type::temp_buffer().size(),
                           qi::omit[ "HTTP/" >> qi::int_ >> '.' >> qi::int_ >> +qi::space] >> qi::int_, base_type::status_code())) {
                // TODO: ちゃんとしたエラーカテゴリを定義する
                on_error(boost::system::error_code(EINVAL, boost::system::generic_category()), "qi::parse request_line");
                return;
            }

            base_type::temp_buffer().consume(size);
            boost::asio::async_read_until(
                socket_, base_type::temp_buffer(), "\r\n\r\n",
                std::bind(
                    &client_socket::on_read_header,
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
            auto beg = boost::asio::buffer_cast<char const *>(base_type::temp_buffer().data());
            auto end = beg + size;

            namespace qi = boost::spirit::qi;
            if (!qi::parse(beg, end, (
                          *(qi::char_ - ':') >> ':' >> qi::omit[ *qi::space ] >> *(qi::char_ - "\r\n")
                           ) % "\r\n" >> "\r\n", header)) {
                // TODO: ちゃんとしたエラーカテゴリを定義する
                on_error(boost::system::error_code(EINVAL, boost::system::generic_category()), "qi::parse request_header");
                return;
            }

            base_type::temp_buffer().consume(size);
            auto it = header.find("Location");
            if (it != header.end()) {
                on_move(it->second);
                return;
            }

            base_type::buffer_.consume(base_type::buffer_.size());
            std::ostream(&this->buffer_).write(boost::asio::buffer_cast<char const *>(base_type::temp_buffer().data()), base_type::temp_buffer().size());
            base_type::temp_buffer().consume(base_type::temp_buffer().size());

            it = header.find("Connection");
            if (it != header.end()) {
                base_type::is_keep_alive_ = boost::iequals(it->second, "keep-alive");
            }

            it = header.find("Content-Encoding");
            if (it != header.end()) {
                if (boost::iequals(it->second, "gzip")) base_type::set_gzip();
                if (boost::iequals(it->second, "zlib")) base_type::set_zlib();
            }

            it = header.find("Transfer-Encoding");
            if (it != header.end() && boost::iequals(it->second, "chunked")) {
                boost::asio::async_read_until(
                    socket_, base_type::buffer_, "\r\n",
                    std::bind(
                        &client_socket::on_read_chunked_size,
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
                            &client_socket::on_read_sized_content,
                            this->shared_from_this(),
                            std::placeholders::_1, std::placeholders::_2
                        )
                    );
                } else {
                    size = -1;
                    boost::asio::async_read(
                        socket_, base_type::buffer_,
                        std::bind(
                            &client_socket::on_read_sized_content,
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
                        &client_socket::on_read_chunked_content,
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
                    &client_socket::on_read_chunked_size,
                    this->shared_from_this(),
                    std::placeholders::_1, std::placeholders::_2
                )
            );
        } else {
            on_error(error, "on_read_chunked_content");
        }
    }

    template <typename String>
    void on_move(String const & loc)
    {
        char const * beg = boost::asio::buffer_cast<char const *>(base_type::buffer_.data());
        char const * end = beg + base_type::buffer_.size();
        auto & header = base_type::get_header();  // result_->header_ を 前回送信したリクエストヘッダーのバッファに流用する

        namespace qi = boost::spirit::qi;
        qi::symbols<char, int> sym;
        int req, maj, min;
        sym.add("GET", 1)("POST", 2);

        // 前回送信したリクエストラインからメソッドを抽出する
        qi::parse(beg, end,
                  sym  // http method
                  >> qi::omit[ *qi::space >> +(qi::char_ - qi::space) >> *qi::space ]  // url
                  >> "HTTP/" >> qi::int_ >> '.' >> qi::int_ >> "\r\n"  // HTTP/1.1
                  , req, maj, min);
        // status_code が 303 のときは、必ず GET
        if (base_type::status_code() == 303)
            req = 1;

        // 転送先URLを作成
        String url;
        if ((loc.empty() || loc[0] == '/')) {
            // Location: が / から始まるときは、同一ホスト内の移動を解釈する
            scheme_name(this, url);
            // HACK: HTTP/1.1 のときは、送信時に Host ヘッダーを最初にセットしているので、 beg は Host ヘッダーの位置のはず
            if (!qi::parse(beg, end, "Host:" >> qi::omit[ *qi::space ] >> *(qi::char_ - '\r') >> "\r\n", url)) {
                on_error(boost::system::error_code(EINVAL, boost::system::generic_category()), "qi::parse request_header");
                return;
            }
            port_name(this, url);
        } else {
            beg = std::find(beg, end, '\n')+1;
        }
        url += loc;
        header.clear(); // loc の内容が消えるので注意

        // 前回送信したリクエストヘッダー（Host を除く）を header に格納
        qi::parse(beg, end, (
                      *(qi::char_ - ':') >> ':' >> qi::omit[ *qi::space ] >> *(qi::char_ - "\r\n")
                  ) % "\r\n" >> "\r\n", header);
        base_type::buffer_.consume(beg - boost::asio::buffer_cast<char const *>(base_type::buffer_.data()));  // content だけ残す

        // 新しいソケットで再接続
        std::shared_ptr<base_type> socket;
        switch(req) {
            case 1:
                socket = client_->do_connect(detail::get(), uri(url, boost::blank(), header), retry_, false);
                break;
            case 2:
                socket = client_->do_connect(detail::non_encoded_post<buffer_type const &>(base_type::buffer_), uri(url, boost::blank(), header), retry_, false);
                break;
        }
        base_type::temp_buffer().consume(base_type::temp_buffer().size());

        // 今のソケットに、新しいソケットを付け替える
        base_type::move_start(*socket);
    }

    template <typename String>
    void scheme_name(typename Client::http_socket const *, String & url) const
    {
        url += "http://";
    }

    template <typename String>
    void scheme_name(typename Client::https_socket const *, String & url) const
    {
        url += "https://";
    }

    template <typename String>
    void port_name(typename Client::http_socket const *, String & url) const
    {
        int port = base_type::endpoint_.port();
        if (port != 80) {
            url += ':';
            url += std::to_string(port);
        }
    }

    template <typename String>
    void port_name(typename Client::https_socket const *, String & url) const
    {
        int port = base_type::endpoint_.port();
        if (port != 443) {
            url += ':';
            url += std::to_string(port);
        }
    }

    void on_error(boost::system::error_code const & error, char const * func)
    {
        (void) func;
        timer_.cancel();
        //std::cout << "on_error " << error.message() << ' ' << func << std::endl;
        base_type::callback(error, socket_.get_io_service());
    }

    void on_wait(boost::system::error_code const & error)
    {
        if (!error) {
            lowest_layer_socket(socket_).cancel();
        }
    }

    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context & ctx)
    {
        // The verify callback can be used to check whether the certificate that is
        // being presented is valid for the peer. For example, RFC 2818 describes
        // the steps involved in doing this for HTTPS. Consult the OpenSSL
        // documentation for more details. Note that the callback is called once
        // for each certificate in the certificate chain, starting from the root
        // certificate authority.

        // In this example we will simply print the certificate's subject name.
        char subject_name[256];
        X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
        X509_NAME_oneline(X509_get_subject_name(cert), subject_name, sizeof(subject_name));
        //std::cout << "Verifying " << subject_name << std::endl;
        return preverified;
    }

public:
    void async_keep_alive()
    {
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(&buffer1_, 1),
            std::bind(
                &client_socket::on_keep_alive,
                this
            )
        );
    }

private:
    void on_keep_alive()
    {
        if (reuse_) {
            lowest_layer_socket(reuse_->socket_) = boost::move(lowest_layer_socket(socket_));
            reuse_->on_connect1(boost::system::error_code());
        } else {
            client_->erase(this);
        }

        delete this;
    }

    void async_reuse(client_socket * socket)
    {
        reuse_ = socket->shared_from_this();
        lowest_layer_socket(socket_).cancel();
    }

private:
    Client * client_;
    socket_type socket_;
    timer_type timer_;
    int retry_;
    std::atomic<bool> is_ready_;
    char buffer1_;
    std::shared_ptr<client_socket> reuse_;
};


} } }
