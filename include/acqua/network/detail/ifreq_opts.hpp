#pragma once

#include <acqua/config.hpp>

extern "C" {
#include <sys/ioctl.h>
#include <net/if.h>
}

#include <boost/system/error_code.hpp>
#include <acqua/network/linklayer_address.hpp>

namespace acqua { namespace network { namespace detail {

class ifreq_opts
{
public:
    ACQUA_DECL explicit ifreq_opts(char const * if_name);

    ACQUA_DECL ~ifreq_opts();

    ACQUA_DECL int get_index(boost::system::error_code & ec);

    ACQUA_DECL int get_mtu(boost::system::error_code & ec);

    ACQUA_DECL void set_mtu(int mtu, boost::system::error_code & ec);

    ACQUA_DECL linklayer_address get_lladdr(boost::system::error_code & ec);

    ACQUA_DECL void set_lladdr(linklayer_address const & lladdr, boost::system::error_code & ec);

private:
    ACQUA_DECL bool open_once(boost::system::error_code & ec);

private:
    int fd_;
    ::ifreq ifr_;
};

} } }

#include <acqua/network/detail/impl/ifreq_opts.ipp>
