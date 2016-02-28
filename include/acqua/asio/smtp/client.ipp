#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <acqua/asio/smtp/client.hpp>
#include <acqua/iostreams/base64_filter.hpp>
#include <acqua/iostreams/crypto/hmac_filter.hpp>
#include <acqua/utility/hexstring.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace acqua { namespace asio { namespace smtp {

struct client::impl
{
    explicit impl(boost::asio::io_service & io_service)
        : context_(boost::asio::ssl::context::tlsv12_client), socket_(io_service, context_), resolver_(io_service), timer_(io_service) {}

    void connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv, boost::system::error_code & ec)
    {
        boost::asio::async_connect(socket_.lowest_layer(), resolver_.resolve(boost::asio::ip::tcp::resolver::query(host, serv)), yield[ec]);
        if (ec) return;
        if (dump_) *dump_ << '!' << socket_.lowest_layer().remote_endpoint(ec) << std::endl;
    }

    void send_message(boost::asio::yield_context yield, std::string const & sendbuf, boost::system::error_code & ec)
    {
        if (dump_) {
            *dump_ << std::endl << '>';
            dump_->write(sendbuf.c_str(), static_cast<std::streamsize>(sendbuf.size()-2));
            *dump_ << std::endl;
        }
        enable_tls_
            ? boost::asio::async_write(socket_, boost::asio::buffer(sendbuf), yield[ec])
            : boost::asio::async_write(socket_.next_layer(), boost::asio::buffer(sendbuf), yield[ec]);
    }

    void send_data(boost::asio::yield_context yield, std::istream & is, boost::system::error_code & ec)
    {
        if (dump_) {
            *dump_ << std::endl << '>';
        }
        boost::asio::streambuf sendbuf;
        auto mbuf = sendbuf.prepare(4096);
        std::streamsize size;
        while(is.read(boost::asio::buffer_cast<char *>(mbuf), 4096), (size = is.gcount()) > 0) {
            if (dump_) {
                if (verbose_) {
                    dump_->write(boost::asio::buffer_cast<char const *>(mbuf), size);
                } else {
                    *dump_ << '.';
                }
            }
            enable_tls_
                ? boost::asio::async_write(socket_, boost::asio::buffer(mbuf, static_cast<std::size_t>(size)), yield[ec])
                : boost::asio::async_write(socket_.next_layer(), boost::asio::buffer(mbuf, static_cast<std::size_t>(size)), yield[ec]);
            if (ec) break;
        }

        if (dump_) {
            *dump_ << std::endl;
        }
    }

    uint receive_code(boost::asio::yield_context yield, boost::system::error_code & ec)
    {
        std::size_t size = enable_tls_
            ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield[ec])
            : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield[ec]);
        if (ec) return 0;

        char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        uint res = parse_response_code(data, size, ec);
        recvbuf_.consume(size);
        return res;
    }

    uint receive_base64(boost::asio::yield_context yield, std::string & message, boost::system::error_code & ec)
    {
        message.clear();

        std::size_t size = enable_tls_
            ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield[ec])
            : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield[ec]);
        if (ec) return 0;

        char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        uint res = parse_response_code(data, size, ec);
        if (res > 0) {
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_decoder());
            out.push(boost::iostreams::back_inserter(message));
            out.write(data+4, static_cast<std::streamsize>(size-6));
        }
        recvbuf_.consume(size);
        return res;
    }

    uint receive_ehlo(boost::asio::yield_context yield, bool & use_starttls, boost::system::error_code & ec)
    {
        use_starttls = false;

        uint res;
        bool in_progress;
        do {
            std::size_t size = enable_tls_
                ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield[ec])
                : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield[ec]);
            if (ec) return 0;

            char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
            res = parse_response_code(data, size, ec);
            in_progress = (res > 0 && data[3] == '-');
            if (res > 0) {
                std::string message(data+4, size-6);
                if (boost::algorithm::istarts_with(message, "AUTH")) {
                    if (boost::algorithm::ifind_first(message, "PLAIN"))
                        enable_auth_plain_ = true;

                    if (boost::algorithm::ifind_first(message, "LOGIN"))
                        enable_auth_login_ = true;

                    if (boost::algorithm::ifind_first(message, "CRAM-MD5"))
                        enable_auth_cram_md5_ = true;
                }
                if (!enable_tls_ && starttls_ && boost::algorithm::istarts_with(message, "STARTTLS"))
                    use_starttls = true;
            }
            recvbuf_.consume(size);
        } while(in_progress);

        return res;
    }

    uint login(boost::asio::yield_context yield, std::string const & user, std::string const & pass, boost::system::error_code & ec)
    {
        uint res = 0;
        if (enable_auth_cram_md5_) {
            res = auth_cram_md5(yield, user, pass, ec);
            if (res == 0 || !ec) return res;
        }

        if (plaintext_password_) {
             if (enable_auth_plain_) {
                ec.clear();
                res = auth_plain(yield, user, pass, ec);
                if (!ec) return res;
            } else if (enable_auth_login_) {
                ec.clear();
                res = auth_plain(yield, user, pass, ec);
                if (!ec) return res;
            }
        }

        return res;
    }

    uint start_tls(boost::asio::yield_context yield, boost::system::error_code & ec)
    {
        enable_tls_ = false;

        send_message(yield, "STARTTLS\r\n", ec);
        if (ec) return 0;

        uint res = receive_code(yield, ec);
        if (ec) return res;

        socket_.async_handshake(boost::asio::ssl::stream_base::client, yield[ec]);
        if (ec) return 0;

        enable_tls_ = true;
        return res;
    }

    void disconnect()
    {
        boost::system::error_code ec;
        timer_.cancel(ec);
        socket_.lowest_layer().close(ec);
    }

private:
    uint parse_response_code(char const * data, std::size_t size, boost::system::error_code & ec)
    {
        if (dump_) {
            *dump_ << '<';
            dump_->write(data, static_cast<std::streamsize>(size)-2);
            *dump_ << std::endl;
        }

        int res = 0;  // あとで uint に変換するので、負の数は使わない
        if (size >= 6 && std::isdigit(data[0]) && std::isdigit(data[1]) && std::isdigit(data[2])) {
            res =  (data[0] - '0');
            res *= 10;
            res += (data[1] - '0');
            res *= 10;
            res += (data[2] - '0');
            if (res >= 400) {
                // TODO: エラーコード
                ec = make_error_code(boost::system::errc::protocol_error);
            }
        } else {
            ec = make_error_code(boost::system::errc::protocol_error);
        }

        return static_cast<uint>(res);
    }

    uint auth_plain(boost::asio::yield_context yield,
                    std::string const & user, std::string const & pass, boost::system::error_code & ec)
    {
        do {
            std::string sendbuf{"AUTH PLAIN "};
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << user << '\0' << user << '\0' << pass;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            send_message(yield, sendbuf, ec);
            if (ec) return 0;
        } while(0);

        return receive_code(yield, ec);
    }

    uint auth_login(boost::asio::yield_context yield,
                    std::string const & user, std::string const & pass, boost::system::error_code & ec)
    {
        send_message(yield, "AUTH LOGIN\r\n", ec);
        if (ec) return 0;

        uint res = receive_code(yield, ec);
        if (ec) return res;

        do {
            std::string sendbuf;
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << user;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            send_message(yield, sendbuf, ec);
            if (ec) return 0;
        } while(0);

        res = receive_code(yield, ec);
        if (ec) return res;

        do {
            std::string sendbuf;
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << pass;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            send_message(yield, sendbuf, ec);
            if (ec) return 0;
        } while(0);

        return receive_code(yield, ec);

    }

    uint auth_cram_md5(boost::asio::yield_context yield,
                       std::string const & user, std::string const & pass, boost::system::error_code & ec)
    {
        send_message(yield, "AUTH CRAM-MD5\r\n", ec);
        if (ec) return 0;

        std::string buf;
        uint res = receive_base64(yield, buf, ec);
        if (ec) return res;

        do {
            char md5buf[16];
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::hmac_md5_filter(md5buf, pass));
            out.push(boost::iostreams::null_sink());
            out << buf;
            out.reset();

            buf.clear();
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(buf));
            out << user << ' ' << acqua::hexstring(md5buf);
            out.reset();
            buf += '\r'; buf += '\n';
            send_message(yield, buf, ec);
            if (ec) return 0;
        } while(0);

        return receive_code(yield, ec);
    }

private:
    boost::asio::ssl::context context_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::steady_timer timer_;
    boost::asio::streambuf recvbuf_;
    bool enable_tls_ = false;
    bool enable_auth_plain_ = false;
    bool enable_auth_login_ = false;
    bool enable_auth_cram_md5_ = false;

public:
    std::ostream * dump_ = nullptr;
    bool verbose_ = false;
    bool plaintext_password_ = true;
    bool starttls_ = true;
};


client::client(boost::asio::io_service & io_service)
    : impl_(new impl(io_service)) {}

uint client::connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv)
{
    boost::system::error_code ec;
    uint res = connect(yield, host, serv, ec);
    boost::asio::detail::throw_error(ec, "connect");
    return res;
}

uint client::connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv, boost::system::error_code & ec)
{
    impl_->connect(yield, host, serv, ec);
    if (ec) return 0;

    return impl_->receive_code(yield, ec);
}

uint client::helo(boost::asio::yield_context yield, std::string const & hostname)
{
    boost::system::error_code ec;
    uint res = helo(yield, ec, hostname);
    boost::asio::detail::throw_error(ec, "helo");
    return res;
}

uint client::helo(boost::asio::yield_context yield, boost::system::error_code & ec, std::string const & hostname)
{
    do {
        std::string sendbuf{"HELO "};
        sendbuf += (hostname.empty() ? boost::asio::ip::host_name() : hostname);
        sendbuf += '\r'; sendbuf += '\n';
        impl_->send_message(yield, sendbuf, ec);
        if (ec) return 0;
    } while(0);

    return impl_->receive_code(yield, ec);
}

uint  client::ehlo(boost::asio::yield_context yield, std::string const & hostname)
{
    boost::system::error_code ec;
    uint res = ehlo(yield, ec, hostname);
    boost::asio::detail::throw_error(ec, "ehlo");
    return res;
}

uint client::ehlo(boost::asio::yield_context yield, boost::system::error_code & ec, std::string const & hostname)
{
    std::string sendbuf{"EHLO "};
    sendbuf += (hostname.empty() ? boost::asio::ip::host_name() : hostname);
    sendbuf += '\r'; sendbuf += '\n';
    impl_->send_message(yield, sendbuf, ec);
    if (ec) return 0;

    bool use_starttls;
    uint res = impl_->receive_ehlo(yield, use_starttls, ec);
    if (use_starttls) {
        res = impl_->start_tls(yield, ec);
        if (ec) return res;

        impl_->send_message(yield, sendbuf, ec);
        if (ec) return 0;
        res = impl_->receive_ehlo(yield, use_starttls, ec);
    }
    return res;
}

uint client::login(boost::asio::yield_context yield, std::string const & user, std::string const & pass)
{
    boost::system::error_code ec;
    uint res = login(yield, user, pass, ec);
    boost::asio::detail::throw_error(ec, "login");
    return res;
}

uint client::login(boost::asio::yield_context yield, std::string const & user, std::string const & pass, boost::system::error_code & ec)
{
    return impl_->login(yield, user, pass, ec);
}

uint client::mail(boost::asio::yield_context yield, std::string const & address)
{
    boost::system::error_code ec;
    uint res = mail(yield, address, ec);
    boost::asio::detail::throw_error(ec, "mail");
    return res;
}

uint client::mail(boost::asio::yield_context yield, std::string const & address, boost::system::error_code & ec)
{
    do {
        std::string sendbuf{"MAIL FROM: "};
        sendbuf += '<';
        sendbuf += address;
        sendbuf += '>'; sendbuf += '\r'; sendbuf += '\n';
        impl_->send_message(yield, sendbuf, ec);
        if (ec) return 0;
    } while(0);

    return impl_->receive_code(yield, ec);
}

uint client::rcpt(boost::asio::yield_context yield, std::string const & address)
{
    boost::system::error_code ec;
    uint res = rcpt(yield, address, ec);
    boost::asio::detail::throw_error(ec, "rcpt");
    return res;
}

uint client::rcpt(boost::asio::yield_context yield, std::string const & address, boost::system::error_code & ec)
{
    do {
        std::string sendbuf{"RCPT TO: "};
        sendbuf += '<';
        sendbuf += address;
        sendbuf += '>'; sendbuf += '\r'; sendbuf += '\n';
        impl_->send_message(yield, sendbuf, ec);
        if (ec) return 0;
    } while(0);

    return impl_->receive_code(yield, ec);
}

uint client::data(boost::asio::yield_context yield, std::istream & is)
{
    boost::system::error_code ec;
    uint res = data(yield, is, ec);
    boost::asio::detail::throw_error(ec, "data");
    return res;
}

uint client::data(boost::asio::yield_context yield, std::istream & is, boost::system::error_code & ec)
{
    impl_->send_message(yield, "DATA\r\n", ec);
    if (ec) return 0;

    uint res = impl_->receive_code(yield, ec);
    if (ec) return res;

    impl_->send_data(yield, is, ec);
    if (ec) return 0;

    return impl_->receive_code(yield, ec);
}

uint client::quit(boost::asio::yield_context yield)
{
    boost::system::error_code ec;
    uint res = quit(yield, ec);
    boost::asio::detail::throw_error(ec, "quit");
    return res;
}

uint client::quit(boost::asio::yield_context yield, boost::system::error_code & ec)
{
    impl_->send_message(yield, "QUIT\r\n", ec);
    if (ec) return 0;

    uint res = impl_->receive_code(yield, ec);
    impl_->disconnect();;
    return res;
}

void client::dump_socketstream(std::ostream & os, bool verbose)
{
    impl_->dump_ = &os;
    impl_->verbose_ = verbose;
}

void client::disable_plaintext_password()
{
    impl_->plaintext_password_ = false;
}

void client::disable_starttls()
{
    impl_->starttls_ = false;
}

} } }
