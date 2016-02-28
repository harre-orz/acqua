#include <acqua/asio/smtp/client.hpp>
#include <iostream>
#include <sstream>

int main(void)
{
    boost::asio::io_service io_service;

    boost::asio::spawn(io_service, [&io_service](boost::asio::yield_context yield) {
        std::istringstream iss("hello world\r\n.\r\n");
        boost::system::error_code ec;
        acqua::asio::smtp::client smtp(io_service);

        smtp.dump_socketstream(std::cout);
        // smtp.disable_starttls();
        // smtp.disable_plaintext_password();

        smtp.connect(yield, "localhost", "587");
        smtp.ehlo(yield, "localhost");
        smtp.login(yield, "admin", "password");
        smtp.mail(yield, "harre@localhost");
        smtp.rcpt(yield, "harre@localhost");
        smtp.data(yield, iss);
        smtp.quit(yield);
    });
    io_service.run();
}
