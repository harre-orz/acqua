#include <acqua/asio/netlink_listener.hpp>

class NetlinkListener : public acqua::asio::netlink_listener<NetlinkListener>
{
public:
    class category : acqua::asio::netlink_link_tag, acqua::asio::netlink_neighor_tag {};

    explicit NetlinkListener(boost::asio::io_service & io_service)
        : NetlinkListener::base_type(io_service) {}
    
    void on_link(std::string const & ifname, acqua::network::linklayer_address const & addr, int type, int flags)
    {
        std::cout << "link " << ifname  << " " << addr << " type " << type << " flags " << flags << std::endl; 
    }

    void on_neighbor(acqua::network::internet4_address const & addr, acqua::network::linklayer_address const & ll, uint)
    {
       std::cout << addr << ' '<< ll << std::endl;
    }

    void on_neighbor(acqua::network::internet6_address const & addr, acqua::network::linklayer_address const & ll, uint)
    {
       std::cout << "neighor " << addr << ' '<< ll << std::endl;
    }
};

int main()
{
    boost::asio::io_service io_service;
    NetlinkListener netlink(io_service);
    boost::system::error_code ec;
    netlink.start(ec);
    netlink.get_neighor_v4(ec);
    io_service.run();
}
