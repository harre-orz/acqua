#include <iostream>
#include <acqua/asio/pseudo_terminal.hpp>

char buffer[4096];

int main()
{
    boost::asio::io_service io_service;
    acqua::asio::pseudo_terminal_master ptm(io_service, acqua::asio::pseudo_terminal_master::with_open);
    acqua::asio::pseudo_terminal_slave pts(io_service, ptm);
}
