#include <iostream>
#include <acqua/asio/pseudo_terminal.hpp>

char buffer[4096];

void on_read(boost::system::error_code const & error, std::size_t size, acqua::asio::pseudo_terminal_master & ptm)
{
    if (!error) {
        std::cout.write(buffer, size);
        ptm.async_read_some(boost::asio::buffer(buffer, sizeof(buffer)),
                        std::bind(&on_read, std::placeholders::_1, std::placeholders::_2, std::ref(ptm)));
    }

}

int main()
{
    boost::asio::io_service io_service;
    acqua::asio::pseudo_terminal_master ptm(io_service, acqua::asio::pseudo_terminal_master::with_open);
    std::cout << ptm.slave_name() << std::endl;
    ptm.async_read_some(boost::asio::buffer(buffer, sizeof(buffer)),
                        std::bind(&on_read, std::placeholders::_1, std::placeholders::_2, std::ref(ptm)));
    io_service.run();
}
