#include <acqua/website/http_client.hpp>
#include <acqua/website/wget.hpp>

static bool volatile mark;

int main(int,  char **)
{
    boost::asio::io_service io_service;
    acqua::website::http_client client(io_service, mark);
    acqua::website::wget(client, "http://localhost/");
}
