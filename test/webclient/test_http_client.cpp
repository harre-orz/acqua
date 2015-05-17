#include <acqua/webclient/http_client.hpp>
#include <acqua/webclient/wget.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

static int s_count = 0;
static std::mutex s_mutex;


int main(int,  char ** argv)
{
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);
    ctx.set_default_verify_paths();

    boost::thread_group tg;
    for(int i = 0; i < 8; ++i)
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    acqua::webclient::http_client http(io_service, ctx, std::atoi(argv[3]));

    int n = s_count = std::atoi(argv[2]);
    for(int i = 0; i < n; ++i) {
        acqua::webclient::wget(http, argv[1], [&io_service](boost::system::error_code const & error, acqua::webclient::http_client::result & res) {
                std::lock_guard<decltype(s_mutex)> lock(s_mutex);
                --s_count;

                if (!error) {
                    //std::cout << client.use_count() << ' ' << client.keep_count() << ' ' << s_count << std::endl;
                    std::cout << res << std::endl;
                } else {
                    std::cout << error << ' ' << error.message() << std::endl;
                }

                if (s_count == 0) {
                    io_service.stop();
                }
            });
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    tg.join_all();
}
