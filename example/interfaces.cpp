#include <acqua/network/interface.hpp>

using namespace acqua::network;

int main()
{
    std::cout << "---- interfaces ----" << std::endl;
    int i = 0;
    for(auto it = interface::begin(); it != interface::end(); ++it) {
        std::cout << ++i << ": ";
        it->dump(std::cout);
        std::cout << std::endl;
    }

    std::cout << "---- change interface setting, please input number: ";
    int n;
    std::cin >> n;

    i = 0;
    interface::iterator iif;
    for(auto it = interface::begin(); it != interface::end(); it++) {
        if(++i == n) {
            iif = it;
            break;
        }
    }

    iif->dump(std::cout);
    std::cout << std::endl
              << "1: change mtu from " << iif->mtu() << std::endl
              << "2: change lladdr from " << iif->physical_address() << std::endl
        ;
    std::cin >> n;
    boost::system::error_code ec;
    switch(n) {
        case 1: {
            std::cout << "please input mtu: ";
            int mtu;
            std::cin >> mtu;
            iif->mtu(n, ec);
            break;
        }
        case 2: {
            std::cout << "please input lladdr: ";
            std::string lladdr;
            std::cin >> lladdr;
            auto ll_addr = linklayer_address::from_string(lladdr, ec);
            if (ec) break;
            iif->physical_address(ll_addr, ec);
            break;
        }
    }
    std::cout << "error: " << ec << " " << ec.message() << std::endl;
}
