#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/network/internet6_address.hpp>
#include <boost/asio/ip/address_v6.hpp>


BOOST_AUTO_TEST_SUITE(internet6_address)

using acqua::network::internet6_address;
using boost::asio::ip::address_v6;
using boost::system::error_code;

BOOST_AUTO_TEST_CASE(internet6_address__construct)
{
    BOOST_CHECK_EQUAL(internet6_address(), internet6_address::any());
    char addr[] { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
    BOOST_CHECK_EQUAL(internet6_address(addr), internet6_address::from_string("::1"));
}

BOOST_AUTO_TEST_CASE(internet6_address__increment)
{
    internet6_address addr;

    ++addr;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::1"));

    addr++;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::2"));

    BOOST_CHECK_EQUAL(++internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), internet6_address());
}

BOOST_AUTO_TEST_CASE(internet6_address__decrement)
{
    internet6_address addr = internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

    --addr;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"));

    addr--;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffd"));

    BOOST_CHECK_EQUAL(--internet6_address(), internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
}

BOOST_AUTO_TEST_CASE(internet6_address__add)
{
    internet6_address addr;

    addr += 0x10000;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::1:0"));
    BOOST_CHECK_EQUAL(addr + 0xff, internet6_address::from_string("::1:ff"));
}

BOOST_AUTO_TEST_CASE(internet6_address__sub)
{
    internet6_address addr = internet6_address::from_string("::10:0");

    addr -= 0x10000;
    BOOST_CHECK_EQUAL(addr, internet6_address::from_string("::f:0"));
    BOOST_CHECK_EQUAL(addr - 0xff, internet6_address::from_string("::f:1"));
}

BOOST_AUTO_TEST_CASE(internet6_address__compare)
{
    internet6_address addr1 = internet6_address::from_string("fe80::1");
    internet6_address addr2 = internet6_address::from_string("fe80::2");
    BOOST_CHECK_EQUAL(addr1 == addr1, true);
    BOOST_CHECK_EQUAL(addr1 != addr2, true);
    BOOST_CHECK_EQUAL(addr1 < addr2, true);
    BOOST_CHECK_EQUAL(addr1 >= addr2, false);
}

BOOST_AUTO_TEST_CASE(internet6_address__boost_asio_compatible)
{
    BOOST_CHECK_EQUAL(internet6_address::from_string("ff::"), address_v6::from_string("ff::"));
    BOOST_CHECK_EQUAL(internet6_address::from_string("ff::").to_bytes() == address_v6::from_string("ff::").to_bytes(), true);
    BOOST_CHECK_EQUAL(internet6_address::from_string("ff::").to_string(), address_v6::from_string("ff::").to_string());
}

BOOST_AUTO_TEST_CASE(internet6_address__string)
{
    BOOST_CHECK_EQUAL(internet6_address().to_string(), "::");
    BOOST_CHECK_EQUAL(internet6_address::from_string("fe80::1").to_string(), "fe80::1");

    error_code ec;
    BOOST_CHECK_EQUAL(internet6_address::from_string("", ec), internet6_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet6_address::from_string("::1.1", ec), internet6_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet6_address::from_string(" fe80::", ec), internet6_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet6_address::from_string("fe80::1:fffff", ec), internet6_address::any());
    BOOST_CHECK_NE(ec, error_code());
}

BOOST_AUTO_TEST_SUITE_END()
