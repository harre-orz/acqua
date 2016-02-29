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

    uint connect(boost::asio::yield_context const & yield, std::string const & host, std::string const & serv)
    {
        boost::asio::async_connect(socket_.lowest_layer(), resolver_.resolve(boost::asio::ip::tcp::resolver::query(host, serv)), yield);
        if (is_error(yield)) return 0;

        if (dump_) {
            boost::system::error_code ec;
            *dump_ << '!' << socket_.lowest_layer().remote_endpoint(ec) << std::endl;
        }
        return 1;
    }

    uint send_message(boost::asio::yield_context const & yield, std::string const & sendbuf)
    {
        if (dump_) {
            *dump_ << std::endl << '>';
            dump_->write(sendbuf.c_str(), static_cast<std::streamsize>(sendbuf.size()-2));
            *dump_ << std::endl;
        }
        enable_tls_
            ? boost::asio::async_write(socket_, boost::asio::buffer(sendbuf), yield)
            : boost::asio::async_write(socket_.next_layer(), boost::asio::buffer(sendbuf), yield);
        return is_error(yield) ? 0 : 1;
    }

    uint send_data(boost::asio::yield_context const & yield, std::istream & is)
    {
        if (dump_) {
            *dump_ << std::endl << '>';
        }
        boost::asio::streambuf sendbuf;
        auto mbuf = sendbuf.prepare(4096);
        std::streamsize size;
        while(is.read(boost::asio::buffer_cast<char *>(mbuf), static_cast<std::streamsize>(boost::asio::buffer_size(mbuf)))
              , (size = is.gcount()) > 0)
        {
            if (dump_) {
                if (verbose_) {
                    dump_->write(boost::asio::buffer_cast<char const *>(mbuf), size);
                } else {
                    *dump_ << '.';
                }
            }
            enable_tls_
                ? boost::asio::async_write(socket_, boost::asio::buffer(mbuf, static_cast<std::size_t>(size)), yield)
                : boost::asio::async_write(socket_.next_layer(), boost::asio::buffer(mbuf, static_cast<std::size_t>(size)), yield);
            if (is_error(yield)) break;
        }

        if (dump_) {
            *dump_ << std::endl;
        }

        return is_error(yield) ? 0 : 1;
    }

    uint receive_code(boost::asio::yield_context const & yield)
    {
        std::size_t size = enable_tls_
            ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield)
            : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield);
        if (is_error(yield)) return 0;

        char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        uint res = parse_response_code(yield, data, size);
        recvbuf_.consume(size);
        return res;
    }

    uint receive_message(boost::asio::yield_context const & yield, std::string & message)
    {
        message.clear();

        std::size_t size = enable_tls_
            ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield)
            : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield);
        if (is_error(yield)) return 0;

        char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        uint res = parse_response_code(yield, data, size);
        if (!is_error(yield)) {
            message.assign(data+4, size-6);
        }
        recvbuf_.consume(size);
        return res;
    }

    uint receive_ehlo(boost::asio::yield_context const & yield, bool & use_starttls)
    {
        use_starttls = false;

        uint res;
        bool in_progress;
        do {
            std::size_t size = enable_tls_
                ? boost::asio::async_read_until(socket_, recvbuf_, "\r\n", yield)
                : boost::asio::async_read_until(socket_.next_layer(), recvbuf_, "\r\n", yield);
            if (is_error(yield)) return 0;

            char const * data = boost::asio::buffer_cast<char const *>(recvbuf_.data());
            res = parse_response_code(yield, data, size);
            in_progress = (res > 0 && data[3] == '-');
            if (res != 0) {
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

    uint login(boost::asio::yield_context const & yield, std::string const & user, std::string const & pass)
    {
        uint res = 0;

        if (enable_auth_cram_md5_) {
            res = auth_cram_md5(yield, user, pass);
            if (res == 0 || !is_error(yield)) return res;
        }

        if (plaintext_password_) {
            if (enable_auth_login_) {
                res = auth_plain(yield, user, pass);
            } else  if (enable_auth_plain_) {
                res = auth_plain(yield, user, pass);
            }
        }

        return res;
    }

    uint start_tls(boost::asio::yield_context const & yield)
    {
        uint res;
        if (!send_message(yield, "STARTTLS\r\n"))
            return 0;

        if (!(res = receive_code(yield)))
            return 0;

        socket_.async_handshake(boost::asio::ssl::stream_base::client, yield);
        if (is_error(yield)) return 0;

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
    bool is_error(boost::asio::yield_context const & yield) const
    {
        return (yield.ec_ && *yield.ec_);
    }


    void set_or_throw_error(boost::system::error_code const & error, boost::asio::yield_context const & yield)
    {
        if (yield.ec_) {
            *yield.ec_ = error;
        } else {
            boost::asio::detail::throw_error(error, "smtp.client");
        }
    }

    uint parse_response_code(boost::asio::yield_context const & yield, char const * data, std::size_t size)
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
                set_or_throw_error(make_error_code(boost::system::errc::protocol_error), yield);
            }
        } else {
            set_or_throw_error(make_error_code(boost::system::errc::protocol_error), yield);
        }

        return static_cast<uint>(res);
    }

    uint auth_plain(boost::asio::yield_context const & yield, std::string const & user, std::string const & pass)
    {
        do {
            std::string sendbuf{"AUTH PLAIN "};
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << user << '\0' << user << '\0' << pass;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            if (!send_message(yield, sendbuf))
                return 0;
        } while(0);

        return receive_code(yield);
    }

    uint auth_login(boost::asio::yield_context const & yield, std::string const & user, std::string const & pass)
    {
        if (!send_message(yield, "AUTH LOGIN\r\n"))
            return 0;

        uint res = receive_code(yield);
        if (res == 0 || res >= 400) return res;

        do {
            std::string sendbuf;
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << user;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            if (!send_message(yield, sendbuf))
                return 0;
        } while(0);

        res = receive_code(yield);
        if (res == 0 || res >= 400) return res;

        do {
            std::string sendbuf;
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_encoder());
            out.push(boost::iostreams::back_inserter(sendbuf));
            out << pass;
            out.reset();
            sendbuf += '\r'; sendbuf += '\n';
            if (!send_message(yield, sendbuf))
                return 0;
        } while(0);
        return receive_code(yield);

    }

    uint auth_cram_md5(boost::asio::yield_context const & yield, std::string const & user, std::string const & pass)
    {
        if (!send_message(yield, "AUTH CRAM-MD5\r\n"))
            return 0;

        std::string buf;
        uint res = receive_message(yield, buf);
        if (res == 0 || res >= 400) return res;

        do {
            char md5buf[16];
            boost::iostreams::filtering_ostream out;
            out.push(acqua::iostreams::base64_decoder());
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
            if (!send_message(yield, buf))
                return 0;
        } while(0);

        return receive_code(yield);
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
    bool plaintext_password_;
    bool starttls_;
};


client::client(boost::asio::io_service & io_service, client_flags flags)
    : impl_(new impl(io_service))
{
    impl_->starttls_ = !(flags & no_starttls);
    impl_->plaintext_password_ = !(flags & no_plaintext_password);
}

uint client::connect(boost::asio::yield_context yield, std::string const & host, std::string const & serv)
{
    if (!impl_->connect(yield, host, serv))
        return 0;

    return impl_->receive_code(yield);
}

uint client::helo(boost::asio::yield_context yield, std::string const & hostname)
{
    do {
        std::string sendbuf{"HELO "};
        sendbuf += (hostname.empty() ? boost::asio::ip::host_name() : hostname);
        sendbuf += '\r'; sendbuf += '\n';
        if (!impl_->send_message(yield, sendbuf))
            return 0;
    } while(0);

    return impl_->receive_code(yield);
}

uint client::ehlo(boost::asio::yield_context yield, std::string const & hostname)
{
    std::string sendbuf{"EHLO "};
    sendbuf += (hostname.empty() ? boost::asio::ip::host_name() : hostname);
    sendbuf += '\r'; sendbuf += '\n';
    if (!impl_->send_message(yield, sendbuf))
        return 0;

    bool use_starttls;
    uint res = impl_->receive_ehlo(yield, use_starttls);
    if (use_starttls) {
        res = impl_->start_tls(yield);
        if (res == 0 || res >= 400)
            return res;

        if (!impl_->send_message(yield, sendbuf))
            return 0;

        res = impl_->receive_ehlo(yield, use_starttls);
    }
    return res;
}

uint client::login(boost::asio::yield_context yield, std::string const & user, std::string const & pass)
{
    return impl_->login(yield, user, pass);
}

uint client::mail(boost::asio::yield_context yield, std::string const & address)
{
    do {
        std::string sendbuf{"MAIL FROM: "};
        sendbuf += '<';
        sendbuf += address;
        sendbuf += '>'; sendbuf += '\r'; sendbuf += '\n';
        if (!impl_->send_message(yield, sendbuf))
            return 0;
    } while(0);

    return impl_->receive_code(yield);
}

uint client::rcpt(boost::asio::yield_context yield, std::string const & address)
{
    do {
        std::string sendbuf{"RCPT TO: "};
        sendbuf += '<';
        sendbuf += address;
        sendbuf += '>'; sendbuf += '\r'; sendbuf += '\n';
        if (!impl_->send_message(yield, sendbuf))
            return 0;
    } while(0);

    return impl_->receive_code(yield);
}

uint client::data(boost::asio::yield_context yield, std::istream & is)
{
    uint res;

    if (!impl_->send_message(yield, "DATA\r\n"))
        return 0;

    if ((res = impl_->receive_code(yield)) != 354)
        return res;

    if (!impl_->send_data(yield, is))
        return 0;

    return impl_->receive_code(yield);
}

uint client::quit(boost::asio::yield_context yield)
{
    uint res;

    if (!impl_->send_message(yield, "QUIT\r\n"))
        return 0;

    if (!(res = impl_->receive_code(yield)))
        return res;

    impl_->disconnect();;
    return res;
}

void client::dump_socketstream(std::ostream & os, bool verbose)
{
    impl_->dump_ = &os;
    impl_->verbose_ = verbose;
}

} } }
