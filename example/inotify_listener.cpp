#include <iostream>
#include <acqua/asio/inotify_listener.hpp>

class InotifyListener
    : public acqua::asio::inotify_listener<InotifyListener>
{
public:
    explicit InotifyListener(boost::asio::io_service & io_service)
        : InotifyListener::base_type(io_service) {}

    void on_open(std::string const & name)
    {
        std::cout << "open " << name << std::endl;
    }

    void on_close(std::string const & name)
    {
        std::cout << "close " << name << std::endl;
    }
};

int main(int argc, char ** argv)
{
    boost::asio::io_service io_service;
    InotifyListener inotify(io_service);

    boost::system::error_code ec;
    inotify.start(ec);

    for(int i = 1; i < argc; ++i)
        inotify.add(argv[i]);

    io_service.run();
}
