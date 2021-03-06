#include <acqua/asio/icmp/pinger_v4.hpp>
#include <acqua/asio/icmp/pinger_v6.hpp>
#include <iostream>
#include <thread>
#include <functional>
#include <boost/bind.hpp>


std::size_t cnt = 0;
std::chrono::steady_clock::duration timeout = std::chrono::seconds(3);
std::chrono::steady_clock::duration interval = std::chrono::seconds(1);

char const * pinger_name(acqua::asio::icmp::pinger_v4 *) { return "v4"; }
char const * pinger_name(acqua::asio::icmp::pinger_v6 *) { return "v6"; }

template <typename Pinger>
void on_check(boost::system::error_code const & error, boost::asio::ip::icmp::endpoint const & host, Pinger * pinger)
{
    if (error) {
        std::cout << pinger_name(pinger) << ' '<< error.message() << std::endl;
        return;
    }

    std::cout << "check " << ++cnt << ' ' << host << std::endl;
    std::this_thread::sleep_for(interval);
    pinger->async_check(host.address().to_string(), timeout, std::bind(&on_check<Pinger>, std::placeholders::_1, std::placeholders::_2, pinger));
}

template <typename Pinger>
void on_search(boost::system::error_code const & error, std::vector<boost::asio::ip::icmp::endpoint> const & remotes, Pinger * pinger)
{
    if (error) {
        std::cout << pinger_name(pinger) << ' ' << error.message() << std::endl;
        return;
    }

    for(auto && ep : remotes) {
        std::cout << "search " << ep << std::endl;
    }
}


int main(int argc, char ** argv)
{
    bool use_v4 = true;
    bool use_v6 = true;

    int ch;
    while((ch = ::getopt(argc, argv, "46t:i:")) > 0) {
        switch(ch) {
            case '4':
                use_v6 = false;
                break;
            case '6':
                use_v4 = false;
                break;
            case 't':
                timeout = std::chrono::milliseconds(static_cast<int>(atof(optarg) * 1000));
                break;
            case 'i':
                interval = std::chrono::milliseconds(static_cast<int>(atof(optarg) * 1000));
                break;
        }
    }
    boost::asio::io_service io_service;
    acqua::asio::icmp::pinger_v4 v4(io_service);
    acqua::asio::icmp::pinger_v6 v6(io_service);
    if (use_v4) {
        v4.start();
        v4.async_check(argv[optind], timeout, std::bind(&on_check<acqua::asio::icmp::pinger_v4>, std::placeholders::_1, std::placeholders::_2, &v4));
        v4.async_search(argv[optind], timeout, std::bind(&on_search<acqua::asio::icmp::pinger_v4>, std::placeholders::_1, std::placeholders::_2, &v4));
    }
    if (use_v6) {
        v6.start();
        v6.async_check(argv[optind], timeout, std::bind(&on_check<acqua::asio::icmp::pinger_v6>, std::placeholders::_1, std::placeholders::_2, &v6));
        v6.async_search(argv[optind], timeout, std::bind(&on_search<acqua::asio::icmp::pinger_v6>, std::placeholders::_1, std::placeholders::_2, &v6));
    }
    io_service.run();
}
