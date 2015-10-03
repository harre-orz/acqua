#pragma once

#include <acqua/network/detail/ifreq_opts.hpp>

namespace acqua { namespace network { namespace detail {

ifreq_opts::ifreq_opts(char const * if_name)
    : fd_(-1)
{
    using namespace std;
    memset(&ifr_, 0, sizeof(ifr_));
    if (if_name) {
        strncpy(ifr_.ifr_name, if_name, sizeof(ifr_.ifr_name));
    }
}

ifreq_opts::~ifreq_opts()
{
    if (fd_ >= 0)
        ::close(fd_);
}

bool ifreq_opts::open_once(boost::system::error_code &ec)
{
    if (fd_ < 0) {
        if ((fd_ = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            ec.assign(errno, boost::system::generic_category());
            return false;
        }
    }
    return true;
}

int ifreq_opts::get_index(boost::system::error_code & ec)
{
    if (!open_once(ec))
        return -1;

    if (::ioctl(fd_, SIOCGIFINDEX, &ifr_) != 0) {
        ec.assign(errno, boost::system::generic_category());
        return -1;
    }

    return ifr_.ifr_ifindex;
}

int ifreq_opts::get_mtu(boost::system::error_code & ec)
{
    if (!open_once(ec))
        return -1;

    if (::ioctl(fd_, SIOCGIFMTU, &ifr_) != 0) {
        ec.assign(errno, boost::system::generic_category());
        return -1;
    }

    return ifr_.ifr_mtu;
}

void ifreq_opts::set_mtu(int mtu, boost::system::error_code & ec)
{
    if (!open_once(ec))
        return;

    ifr_.ifr_mtu = mtu;
    if (::ioctl(fd_, SIOCSIFMTU, &ifr_) != 0) {
        ec.assign(errno, boost::system::generic_category());
        return;
    }
}

linklayer_address ifreq_opts::get_lladdr(boost::system::error_code & ec)
{
    if (!open_once(ec))
        return linklayer_address();

    if (::ioctl(fd_, SIOCGIFHWADDR, &ifr_) != 0) {
        ec.assign(errno, boost::system::generic_category());
        return linklayer_address();
    }

    return linklayer_address(ifr_.ifr_hwaddr.sa_data);
}

void ifreq_opts::set_lladdr(linklayer_address const & lladdr, boost::system::error_code & ec)
{
    if (!open_once(ec))
        return;

    using namespace std;
    ifr_.ifr_hwaddr.sa_family = 1; //ARPHRD_ETHER;
    memcpy(ifr_.ifr_hwaddr.sa_data, &lladdr, 6);
    if (::ioctl(fd_, SIOCSIFHWADDR, &ifr_) != 0) {
        ec.assign(errno, boost::system::generic_category());
    }
}

} } }
