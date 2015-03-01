#pragma once

#include <iostream>
#include <boost/array.hpp>
#include <boost/operators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi.hpp>
#include <acqua/exception/throw_error.hpp>

namespace acqua { namespace network {

/*!
  リンクレイヤーアドレス.

  trivial なデータ型
 */
class linklayer_address
    : boost::totally_ordered<linklayer_address>
    , boost::unit_steppable<linklayer_address>
    , boost::additive2<linklayer_address, long int>
{
public:
    using bytes_type = boost::array<unsigned char, 6>;

    linklayer_address() noexcept
    {
        static_assert(sizeof(*this) == 6, "");
        bytes_.fill(0);
    }

    linklayer_address(linklayer_address const &) noexcept = default;

    linklayer_address(linklayer_address &&) noexcept = default;

    linklayer_address(bytes_type const & bytes) noexcept
        : bytes_(bytes)
    {
    }

    linklayer_address(char const addr[6]) noexcept
    {
        copy_from(addr);
    }

    linklayer_address(unsigned char addr[6]) noexcept
    {
        copy_from(addr);
    }

    linklayer_address(signed char addr[6]) noexcept
    {
        copy_from(addr);
    }

    linklayer_address & operator=(linklayer_address const &) noexcept = default;

    linklayer_address & operator=(linklayer_address &&) noexcept = default;

    bool operator==(linklayer_address const & rhs) const noexcept
    {
        return bytes_ == rhs.bytes_;
    }

    bool operator<(linklayer_address const & rhs) const noexcept
    {
        return bytes_ < rhs.bytes_;
    }

    linklayer_address & operator++() noexcept
    {
        for(auto it = bytes_.rbegin(); ++(*it) == 0x00 && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    linklayer_address & operator+=(long int num) noexcept
    {
        if (num < 0)
            return operator-=(-num);

        for(auto it = bytes_.rbegin(); it != bytes_.rend() && num; ++it) {
            *it += (num & 0xff);
            num >>= 8;
        }

        return *this;
    }

    linklayer_address & operator--() noexcept
    {
        for(auto it = bytes_.rbegin(); --(*it) == 0xff && it != bytes_.rend(); ++it)
            ;
        return *this;
    }

    linklayer_address & operator-=(long int num) noexcept
    {
        if (num < 0)
            return operator+=(-num);

        for(auto it = bytes_.rbegin(); it != bytes_.rend() && num; ++it) {
            *it -= (num & 0xff);
            num >>= 8;
        }

        return *this;
    }

    static linklayer_address any() noexcept
    {
        return linklayer_address();
    }

    static linklayer_address broadcast() noexcept
    {
        bytes_type bytes;
        bytes.fill(255);
        return linklayer_address(bytes);
    }

    static linklayer_address from_string(std::string const & str, boost::system::error_code & ec) noexcept
    {
        return from_string(str.begin(), str.end(), ec);
    }

    static linklayer_address from_string(std::string const & str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str.begin(), str.end(), ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    static linklayer_address from_string(char const * str, boost::system::error_code & ec) noexcept
    {
        return from_string(str, str + std::strlen(str), ec);
    }

    static linklayer_address from_string(char const * str)
    {
        boost::system::error_code ec;
        auto addr = from_string(str, str + std::strlen(str), ec);
        acqua::exception::throw_error(ec, "from_string");
        return addr;
    }

    bool is_unspecified() const noexcept
    {
        return *this == any();
    }

    bytes_type to_bytes() const noexcept
    {
        return bytes_;
    }

    std::string to_string() const
    {
        return boost::lexical_cast<std::string>(*this);
    }

    int to_oui() const noexcept
    {
        int oui = bytes_[2];
        oui += bytes_[1] * 10;
        oui += bytes_[0] * 100;
        return oui;
    }

    template <typename Ch, typename Tr>
    friend std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> & os, linklayer_address const & rhs)
    {
        char buf[3*6];
        std::sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", rhs.bytes_[0], rhs.bytes_[1], rhs.bytes_[2], rhs.bytes_[3], rhs.bytes_[4], rhs.bytes_[5]);
        std::copy_n(buf, 17, std::ostreambuf_iterator<char>(os));
        return os;
    }

    friend std::size_t hash_value(linklayer_address const & rhs) noexcept
    {
        return rhs.hash_func<std::size_t>();
    }

private:
    template <typename T, typename std::enable_if< std::is_integral<T>::value>::type * = nullptr>
    void copy_from(T const * t) noexcept
    {
        reinterpret_cast<std::uint32_t *>(bytes_.data())[0] = reinterpret_cast<std::uint32_t const *>(t)[0];
        reinterpret_cast<std::uint16_t *>(bytes_.data())[4] = reinterpret_cast<std::uint16_t const *>(t)[4];
    }

    template <typename It>
    static linklayer_address from_string(It beg, It end, boost::system::error_code & ec) noexcept
    {
        bytes_type bytes;

        namespace qi = boost::spirit::qi;
        qi::uint_parser<unsigned char, 16, 2, 2> hex;
        qi::rule<It, unsigned char> sep = qi::lit(':') | qi::lit('-');
        if (!qi::parse(beg, end, hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex >> sep >> hex,
                       bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]) || beg != end)
            ec.assign(EAFNOSUPPORT, boost::system::generic_category());
        return linklayer_address(bytes);
    }

    template <typename T, typename std::enable_if<sizeof(T) == 4>::type * = nullptr>
    T hash_func() const noexcept
    {
        return reinterpret_cast<std::uint32_t const *>(bytes_.data())[0]
            ^  reinterpret_cast<std::uint16_t const *>(bytes_.data())[4];
    }

    template <typename T, typename std::enable_if<sizeof(T) == 8>::type * = nullptr>
    T hash_func() const noexcept
    {
        return (reinterpret_cast<std::uint32_t const *>(bytes_.data())[0] << 16)
            +  (reinterpret_cast<std::uint16_t const *>(bytes_.data())[4]);
    }

private:
    bytes_type bytes_;
} __attribute__((__packed__));

} }
