#include <acqua/website/http_client.hpp>
#include <acqua/website/wget.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

static int s_count = 0;
static std::mutex s_mutex;

int main(int argc,  char ** argv)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);
    boost::thread_group tg;
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    acqua::website::http_client client(io_service);
    int n = s_count = std::atoi(argv[2]);
    for(int i = 0; i < n; ++i) {
        acqua::website::wget(client, argv[1], [&io_service](boost::system::error_code const & error, acqua::website::http_client::result & res) {
                std::lock_guard<decltype(s_mutex)> lock(s_mutex);
                if (!error) {
                    //std::cout << res << std::endl;
                } 

                if (error || --s_count == 0) {
                    std::cerr << error.message() << std::endl;
                    io_service.stop();
                }
            });
    }

    tg.join_all();
}
