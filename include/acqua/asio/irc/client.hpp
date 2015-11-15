#pragma once

#include <iostream>
#include <functional>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/parameter.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/asio/irc/parameter.hpp>

namespace acqua { namespace asio { namespace irc {

template <typename Derived>
class client
{
protected:
    using base_type = client;

public:
    explicit client(boost::asio::io_service & io_service)
        : socket_(io_service)
        , resolver_(io_service)
    {
    }

    BOOST_PARAMETER_MEMBER_FUNCTION(
        (void), connect, tag,
        (required (host, *) (nick, *) )
        (optional (port, *, "6667") (pass, *, "") (user, *, nick) (real, *, nick) ) )
    {
        host_ = host;
        nick_ = nick;
        port_ = port;
        pass_ = pass;
        user_ = user;
        real_ = real;

        resolver_.async_resolve(
            boost::asio::ip::tcp::resolver::query(host_, port_),
            std::bind(&base_type::on_resolve, this, std::placeholders::_1, std::placeholders::_2));
    }

    void join(std::string const & channel)
    {
        send_msg("JOIN", channel);
    }

    void say(std::string const & channel, std::string const & msg)
    {
        send_msg("PRIVMSG", channel, msg);
    }

private:
    void on_welcome()
    {
    }

    void on_nickname_inuse()
    {
    }

    void on_join(std::string const &)
    {
    }

    void on_privmsg(std::string const &, std::string const &)
    {
    }

private:
    template <typename Arg1, typename Arg2>
    void send_msg(Arg1 && arg1, Arg2 && arg2)
    {
        std::ostream(&sendbuf_)
            << arg1 << ' ' << arg2 << '\r' << '\n';
        boost::asio::write(socket_, sendbuf_);
    }

    template <typename Arg1, typename Arg2, typename Arg3>
    void send_msg(Arg1 && arg1, Arg2 && arg2, Arg3 && arg3)
    {
        std::ostream(&sendbuf_)
            << arg1 << ' ' << arg2 << ' ' << ':' << arg3 << '\r' << '\n';
        boost::asio::write(socket_, sendbuf_);
    }

    void on_resolve(boost::system::error_code const & error, boost::asio::ip::tcp::resolver::iterator it)
    {
        if (error) {
            return;
        }

        boost::asio::async_connect(socket_, it, std::bind(&base_type::on_connect, this, std::placeholders::_1));
    }

    void on_connect(boost::system::error_code const & error)
    {
        if (error) {
            return;
        }

        boost::asio::async_read_until(
            socket_, recvbuf_, "\r\n",
            std::bind(&base_type::dispatch, this, std::placeholders::_1, std::placeholders::_2));

        std::ostream os(&sendbuf_);
        if (!pass_.empty())
            os << "PASS " << pass_ << "\r\n";
        os <<
            "USER " << user_ << " * 0 " << real_ << "\r\n"
            "NICK " << nick_ << "\r\n";
        boost::asio::write(socket_, sendbuf_);
    }

    void dispatch(boost::system::error_code const & error, std::size_t size)
    {
        if (error) {
            return;
        }

        int code;
        std::string nick;
        std::string targ;
        char const * beg = boost::asio::buffer_cast<char const *>(recvbuf_.data());
        char const * it = beg;
        char const * end = beg + size - 2;
        *const_cast<char *>(end) = 0;
        std::cout << beg << std::endl;

        it = std::find(beg, end, ' ');
        if (it != end)
            ++it;

        if (boost::algorithm::istarts_with(beg, "PING")) {
            send_msg("PONG", it);
        } else {
            namespace qi = boost::spirit::qi;
            qi::symbols<char, int> sym;
            sym.add
                ("001", 1)
                ("353", 353)
                ("JOIN", 1000)
                ("PRIVMSG", 1001);
            if (qi::parse(it, end, sym >> ' ', code)) {
                switch(code) {
                    case 1:  // WELCOME
                        static_cast<Derived *>(this)->on_welcome();
                        break;
                    case 353:  // JOIN
                        if (qi::parse(it, end, *(qi::char_ - qi::space) >> ' ' >> (qi::lit('=') | '*' | '@') >> ' ' >> *(qi::char_ - qi::space), nick, targ))
                            static_cast<Derived *>(this)->on_join(targ);
                        break;
                    case 1000:  // ON_JOIN
                        if (qi::parse(it, end, ':' >> *(qi::char_ - qi::space), targ))
                            static_cast<Derived *>(this)->on_join(targ);
                        break;
                    case 1001:  // ON_PRIVMSG
                        if (qi::parse(it, end, *(qi::char_ - qi::space) >> ' ' >> ':', targ))
                            static_cast<Derived *>(this)->on_privmsg(targ, std::string(it, end));
                        break;
                }
            }
        }

        recvbuf_.consume(size);
        boost::asio::async_read_until(
            socket_, recvbuf_, "\r\n",
            std::bind(&base_type::dispatch, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::streambuf sendbuf_;
    boost::asio::streambuf recvbuf_;
    std::string host_;
    std::string port_;
    std::string nick_;
    std::string pass_;
    std::string user_;
    std::string real_;
};

} } }
