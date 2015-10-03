#include <acqua/network/interface.hpp>

int main()
{
    for(auto it = acqua::network::interface::begin(); it != acqua::network::interface::end(); ++it) {
        std::cout << *it << " mac " << it->physical_address() << std::endl;
    }

}
