#include <acqua/asio/netlink/netlink_listener.hpp>

class NetlinkListener : public acqua::asio::netlink::netlink_listener<NetlinkListener>
{
public:
    class category
        : acqua::asio::netlink::link_tag
        , acqua::asio::netlink::stats_tag
        , acqua::asio::netlink::ifaddr_tag
        , acqua::asio::netlink::neighbor_tag
    {};

    explicit NetlinkListener(boost::asio::io_service & io_service)
        : NetlinkListener::base_type(io_service) {}

    void on_link(std::string const & ifname, acqua::network::linklayer_address const & addr, int type, uint flags)
    {
        std::cout << "link " << ifname  << " " << addr << " type " << type << " flags " << flags << std::endl;
    }

    void on_stats(std::string const & ifname, uint tx_packets, uint tx_bytes, uint rx_packets, uint rx_bytes, int type, uint flags)
    {
        std::cout << "stats " << ifname << " tx_packets=" << tx_packets << " tx_bytes=" << tx_bytes
                  << " rx_packets=" << rx_packets << " rx_bytes=" << rx_bytes << " type " << type << " flags " << flags<< std::endl;
    }

    void on_ifaddr(acqua::network::internet4_address const & addr, std::string const & name, uint prefixlen, uint flags)
    {
        std::cout << "ifaddr v4 " << name << ' ' << addr << '/' << prefixlen << " flags " << flags << std::endl;
    }

    void on_ifaddr(acqua::network::internet6_address const & addr, std::string const & name, uint prefixlen, uint flags)
    {
        std::cout << "ifaddr v6 " << name << ' ' << addr << '/' << prefixlen << " flags " << flags << std::endl;
    }

    void on_neighbor(acqua::network::internet4_address const & addr, acqua::network::linklayer_address const & ll, uint)
    {
        std::cout << "neighbor v4 " << addr << ' '<< ll << std::endl;
    }

    void on_neighbor(acqua::network::internet6_address const & addr, acqua::network::linklayer_address const & ll, uint)
    {
        std::cout << "neighbor v6 " << addr << ' '<< ll << std::endl;
    }
};

int main()
{
    boost::asio::io_service io_service;
    NetlinkListener netlink(io_service);
    boost::system::error_code ec;
    netlink.start(ec);
    io_service.run();
}
