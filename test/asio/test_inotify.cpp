#include <acqua/asio/inotify.hpp>
#include <iostream>

int main(int, char **)
{
    boost::asio::io_service io_service;
    acqua::asio::inotify inotify(io_service);

    inotify.add("test_inotify.cpp");
    inotify.async_wait([](boost::system::error_code const & error, acqua::asio::inotify::iterator it) {
            if (!error) {
                while(it != acqua::asio::inotify::iterator()) {
                    std::cout << it->name() << ' ' << it->in_name() << std::endl;
                    ++it;
                }
            }
    });
    io_service.run();
}
