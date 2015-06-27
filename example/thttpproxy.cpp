#include <memory>
#include <functional>
#include <map>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <acqua/asio/internet_server.hpp>
#include <acqua/asio/proxy_traits.hpp>
#include <acqua/asio/transparent_proxy_traits.hpp>
#include <acqua/asio/read_until.hpp>
#include <acqua/container/sequenced_map.hpp>

class http_proxy
    : public std::enable_shared_from_this<http_proxy>
{
public:
    using socket_type = boost::asio::ip::tcp::socket;

public:
    explicit http_proxy(boost::asio::io_service & io_service)
        : server_socket_(io_service)
        , client_socket_(io_service)
    {
    }

    socket_type & server_socket() { return server_socket_; }
    socket_type & client_socket() { return client_socket_; }

    void start()
    {
        client_socket_.async_connect(destinate_, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, 0));
    }

private:
    void async_proxying(boost::system::error_code const error, std::size_t size)
    {
        if (!error) {
            auto it = header_.end();
            std::lock_guard<decltype(mutex_)> lock(mutex_);

            BOOST_ASIO_CORO_REENTER(coro_) {
                do {
                    is_keep_alive_ = false;

                    // HTTPリクエストヘッダーの終端まで取得
                    BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(server_socket_, server_buffer_, "\r\n\r\n", std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
                    {
                        char const * beg = boost::asio::buffer_cast<char const *>(server_buffer_.data());
                        char const * end = beg + size;

                        namespace qi = boost::spirit::qi;
                        namespace phx = boost::phoenix;

                        // リクエストラインをパース
                        // 一時変数を作らないために、メンバ変数に渡す
                        if (!qi::parse(beg, end,
                                       qi::no_case[ symbol_ ]  // メソッド
                                       >> qi::omit[
                                           *qi::space
                                           >> (+(qi::char_ - qi::space))[phx::bind(&http_proxy::parse_path, this, qi::_1)]  // パス
                                           >> *qi::space
                                       ]
                                       >> "HTTP/" >> qi::int_ >> '.' >> qi::int_
                                       >> qi::omit[*qi::space], method_, main_ver_, sub_ver_)) {
                            on_error(error, "request line");
                            return;
                        }
                        // リクエストヘッダーをパース
                        header_.clear();
                        if (!qi::parse(beg, end, ( +(qi::char_ - ':') >> ':' >> qi::omit[*qi::space] >> *(qi::char_ - "\r\n") ) % "\r\n" >> "\r\n", header_)) {
                            on_error(error, "request header");
                            return;
                        }
                        header_["Connection"] = "close";

                        // プロキシヘッダーを再作成して、サーバに転送
                        it = header_.find("Host");
                        if (it == header_.end()) {
                            on_error(error, "host");
                            return;
                        }
                        std::cout << method_ << " http://" << it->second << path_ << std::endl;

                        std::ostream os(&client_buffer_);
                        os << method_ << ' ';
                        if (!boost::istarts_with(path_, "http://"))
                            os << "http://" << it->second;
                        os << path_ << " HTTP/" << main_ver_ << '.' << sub_ver_ << '\r' << '\n';
                        for(auto const & e : header_)
                            os << e.first << ':' << ' ' << e.second << '\r' << '\n';
                        os << '\r' << '\n';
                    }
                    // HTTPリクエストヘッダーを送信
                    server_buffer_.consume(size);
                    BOOST_ASIO_CORO_YIELD boost::asio::async_write(client_socket_, client_buffer_, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
                    {
                        // request にコンテンツがあれば送信する
                        it = header_.find("Transfer-Encoding");
                        if (it != header_.end() && boost::iequals(it->second, "chunked")) {
                            BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(
                                server_socket_, server_buffer_, "\r\n",
                                std::bind(&http_proxy::async_chunked_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(server_socket_), std::ref(server_buffer_), std::ref(client_socket_), std::ref(client_buffer_))
                            );
                        } else {
                            it = header_.find("Content-Length");
                            if (it != header_.end() && (content_length_ = (std::strtol(it->second.c_str(), nullptr, 10)))) {
                                BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(
                                    server_socket_, server_buffer_, content_length_,
                                    std::bind(&http_proxy::async_content_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(server_buffer_), std::ref(client_socket_), std::ref(client_buffer_))
                                );
                            }
                        }
                    }
                    // HTTPレスポンスヘッダーの終端まで取得
                    server_buffer_.consume(server_buffer_.size());
                    BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(client_socket_, client_buffer_, "\r\n\r\n", std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
                    {
                        char const * beg = boost::asio::buffer_cast<char const *>(client_buffer_.data());
                        char const * end = beg + size;
                        std::ostream(&server_buffer_).write(beg, size);

                        namespace qi = boost::spirit::qi;
                        namespace phx = boost::phoenix;

                        // レスポンスラインをパース
                        if (!qi::parse(beg, end,
                                       "HTTP/" >> qi::int_ >> '.' >> qi::int_
                                       >> +qi::space
                                       >> qi::int_[phx::ref(status_code_) = qi::_1]  // レスポンスコード
                                       >> +qi::space
                                       >> *(qi::char_ - '\r')  // レスポンスメッセージ
                                       >> *qi::space)) {
                            on_error(error, "response line");
                            return;
                        }

                        // レスポンスヘッダーをパース
                        header_.clear();
                        if (!qi::parse(beg, end, ( +(qi::char_ - ':') >> ':' >> qi::omit[*qi::space] >> *(qi::char_ - "\r\n") ) % "\r\n" >> "\r\n", header_)) {
                            on_error(error, "request header");
                            return;
                        }

                        //std::cout << "response: " << status_code_ << std::endl;
                        // for(auto const & e : header_) {
                        //     std::cout << e.first << ' ' << e.second << std::endl;
                        // }
                    }
                    // HTTPレスポンスヘッダーを応答
                    client_buffer_.consume(size);
                    BOOST_ASIO_CORO_YIELD boost::asio::async_write(server_socket_, server_buffer_, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
                    {
                        it = header_.find("Connection");
                        //is_keep_alive_ = (it != header_.end() && boost::iequals(it->second, "keep-alive"));

                        // response にコンテンツがあれば送信する
                        it = header_.find("Transfer-Encoding");
                        if (it != header_.end() && boost::iequals(it->second, "chunked")) {
                            BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(
                                client_socket_, client_buffer_, "\r\n",
                                std::bind(&http_proxy::async_chunked_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(client_socket_), std::ref(client_buffer_), std::ref(server_socket_), std::ref(server_buffer_))
                            );
                        } else {
                            it = header_.find("Content-Length");
                            if (it != header_.end() && (content_length_ = (std::strtol(it->second.c_str(), nullptr, 10)))) {
                                BOOST_ASIO_CORO_YIELD boost::asio::async_read_until(
                                    client_socket_, client_buffer_, content_length_,
                                    std::bind(&http_proxy::async_content_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(client_buffer_), std::ref(server_socket_), std::ref(server_buffer_))
                                );
                            } else if (status_code_ == 200) {
                                is_keep_alive_ = false;
                                // サーバからの切断確認
                                boost::asio::async_read(server_socket_, boost::asio::buffer(&dummy_buffer_, 1), std::bind(&http_proxy::async_check_connected, this->shared_from_this()));
                                while(true) {
                                    BOOST_ASIO_CORO_YIELD boost::asio::async_read(
                                        client_socket_, client_buffer_,
                                        std::bind(&http_proxy::async_unknown_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2)
                                    );
                                }
                            }
                        }
                    }
                } while(is_keep_alive_);
            }
        } else {
            on_error(error, "");
        }
    }

    void async_content_length(boost::system::error_code const & error, std::size_t size, boost::asio::streambuf & recvbuf, socket_type & sendsoc, boost::asio::streambuf & sendbuf)
    {
        if (!error) {
            move_buffer(recvbuf, size, sendbuf);
            boost::asio::async_write(sendsoc, sendbuf, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        }
    }

    void async_chunked_length(boost::system::error_code const & error, std::size_t size, socket_type & recvsoc, boost::asio::streambuf & recvbuf, socket_type & sendsoc, boost::asio::streambuf & sendbuf)
    {
        if (!error) {
            content_length_ = std::strtol(boost::asio::buffer_cast<char const *>(recvbuf.data()), nullptr, 16);
            move_buffer(recvbuf, size, sendbuf);
            if (content_length_ == 0) {
                boost::asio::async_write(sendsoc, sendbuf, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));  // end of chunked contents
            } else {
                content_length_ += 2;
                boost::asio::async_write(sendsoc, sendbuf, std::bind(&http_proxy::async_chunked_length2, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(recvsoc), std::ref(recvbuf), std::ref(sendsoc), std::ref(sendbuf)));
            }
        }
    }

    void async_chunked_length2(boost::system::error_code const& error, std::size_t, socket_type & recvsoc, boost::asio::streambuf & recvbuf, socket_type & sendsoc, boost::asio::streambuf & sendbuf)
    {
        if (!error) {
            boost::asio::async_read_until(recvsoc, recvbuf, content_length_, std::bind(&http_proxy::async_chunked_length3, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(recvsoc), std::ref(recvbuf), std::ref(sendsoc), std::ref(sendbuf)));
        }
    }

    void async_chunked_length3(boost::system::error_code const & error, std::size_t size, socket_type & recvsoc, boost::asio::streambuf & recvbuf, socket_type & sendsoc, boost::asio::streambuf & sendbuf)
    {
        if (!error) {
            move_buffer(recvbuf, size, sendbuf);
            boost::asio::async_write(sendsoc, sendbuf, std::bind(&http_proxy::async_chunked_length4, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(recvsoc), std::ref(recvbuf), std::ref(sendsoc), std::ref(sendbuf)));
        }
    }

    void async_chunked_length4(boost::system::error_code const & error, std::size_t, socket_type & recvsoc, boost::asio::streambuf & recvbuf, socket_type & sendsoc, boost::asio::streambuf & sendbuf)
    {
        if (!error) {
            boost::asio::async_read_until(
                recvsoc, recvbuf, "\r\n",
                std::bind(&http_proxy::async_chunked_length, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2, std::ref(recvsoc), std::ref(recvbuf), std::ref(sendsoc), std::ref(sendbuf))
            );
        }
    }

    void async_unknown_length(boost::system::error_code const &, std::size_t)
    {
        if (client_buffer_.size() > 0) {
            move_buffer(client_buffer_, client_buffer_.size(), server_buffer_);
            boost::asio::async_write(server_socket_, server_buffer_, std::bind(&http_proxy::async_proxying, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        } else {
            async_check_connected();
        }
    }

    void async_check_connected()
    {
        client_socket_.cancel();
        server_socket_.cancel();
    }

    void move_buffer(boost::asio::streambuf & from, std::size_t size, boost::asio::streambuf & to)
    {
        std::streamsize rest;
        char buf[4096];
        std::istream is(&from);
        std::ostream os(&to);

        while(size > sizeof(buf)) {
            is.read(buf, sizeof(buf));
            os.write(buf, sizeof(buf));
            size -= sizeof(buf);
        }
        is.read(buf, size);
        os.write(buf, size);
        // std::ostream(&to).write(boost::asio::buffer_cast<char const *>(from.data()), len);
        // from.consume(len);
    }

    void on_error(boost::system::error_code const & error, char const * location)
    {
        std::cout << "on error(" << location << ')'<< ':' << error.message() << std::endl;
    }

    void parse_path(std::vector<char> const & range) { path_.assign(range.begin(), range.end()); }

private:
    enum method_type {
        GET, HEAD, POST,
    };

    socket_type server_socket_;
    socket_type client_socket_;
    std::mutex mutex_;
    boost::asio::coroutine coro_;
    boost::asio::streambuf client_buffer_;
    boost::asio::streambuf server_buffer_;
    char dummy_buffer_;
    bool is_keep_alive_ = false;
    std::size_t content_length_;
    enum method_type method_;
    std::string path_;
    int main_ver_;
    int sub_ver_;
    int status_code_;
    acqua::container::sequenced_multimap<std::string, std::string> header_;

private:

    static boost::asio::ip::tcp::endpoint destinate_;
    static boost::spirit::qi::symbols<char, enum method_type> symbol_;

    friend std::ostream & operator<<(std::ostream & os, enum method_type rhs)
    {
        static char const * s_str[] = { "GET", "HEAD", "POST" };
        os << s_str[rhs];
        return os;
    }

public:
    static void global_initialize(boost::asio::ip::address_v4 const & address, std::uint16_t port)
    {
        destinate_.address(address);
        destinate_.port(port);
        symbol_.add
            ("get", GET)
            ("head", HEAD)
            ("post", POST)
            ;
    }
};

// 実体
boost::spirit::qi::symbols<char, enum http_proxy::method_type> http_proxy::symbol_;
boost::asio::ip::tcp::endpoint http_proxy::destinate_;


int main(int argc, char ** argv)
{
    http_proxy::global_initialize(boost::asio::ip::address_v4::from_string("127.0.0.1"), 80);

    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);

    acqua::asio::internet_server<http_proxy, acqua::asio::transparent_proxy_traits<http_proxy> > httpproxy(io_service, boost::asio::ip::address_v4::any(), 8080);
    //acqua::asio::internet_server<http_proxy, acqua::asio::proxy_traits<http_proxy> > httpproxy(io_service, boost::asio::ip::address_v4::any(), 8080);

    boost::thread_group tg;
    for(int i = 0; i < 10; ++i)
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    httpproxy.start();
    tg.join_all();
}
