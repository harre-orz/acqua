#pragma once

namespace acqua { namespace network {

class internet4_address;
class intenret6_address;

namespace detail {

template <typename Address>
class prefix_address
{
    using bytes_type = typename Address::bytes_type;

public:
    using address_type = Address;
    using masklen_type = typename Address::masklen_type;

public:
    prefix_address()
        : masklen_(0), address_() {}

    prefix_address(address_type const & address, masklen_type masklen = 0)
    {
    }

    address_type address() const
    {
        return address_;
    }

    masklen_type masklen() const
    {
        return masklen_;
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, prefix_address const & rhs)
    {
        rhs.write(os, rhs.address_, rhs.masklen_);
        return os;
    }

private:
    template <typename Ch, typename Tr, typename Addr,
              typename std::enable_if<std::is_same<Addr, internet4_address>::value>::type * = nullptr>
    static void write(std::basic_ostream<Ch, Tr> & os, Addr const & addr, masklen_type const & mask)
    {
        char buf[4*4+3];
        auto size = std::sprintf(buf, "%d.%d.%d.%d/%d", addr.bytes_[0], addr.bytes_[1], addr.bytes_[2], addr.bytes_[3], mask);
        std::copy_n(buf, size, std::ostreambuf_iterator<Ch>(os));
    }

    template <typename Ch, typename Tr, typename Addr,
              typename std::enable_if<std::is_same<Addr, internet6_address>::value>::type * = nullptr>
    static void write(std::basic_ostream<Ch, Tr> & os, Addr const & addr, masklen_type const & mask)
    {
        char buf[8 + 5 + 4];
        char * end = addr.write(buf);
        *end++ = '/';
        if ((*end = '0' + mask / 100 % 10) != '0') end++;
        if ((*end = '0' + mask /  10 % 10) != '0') end++;
        *end++ = '0' + mask % 10;
        std::copy(buf, end, std::ostreambuf_iterator<Ch>(os));
    }

private:
    masklen_type masklen_;
    address_type address_;
};

} } }
