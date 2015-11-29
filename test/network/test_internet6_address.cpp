#include <acqua/network/internet6_address.hpp>
#include <boost/test/included/unit_test.hpp>
#include <random>

BOOST_AUTO_TEST_SUITE(internet6_address)

using acqua::network::internet6_address;
using boost::asio::ip::address_v6;
using boost::system::error_code;

BOOST_AUTO_TEST_CASE(construct)
{
    BOOST_TEST(internet6_address{} == internet6_address::any());
    BOOST_TEST(internet6_address(typename internet6_address::bytes_type{{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }}) == internet6_address::from_string("::1"));
}

BOOST_AUTO_TEST_CASE(incr)
{
    internet6_address addr;

    ++addr;
    BOOST_TEST(addr == internet6_address::from_string("::1"));

    addr++;
    BOOST_TEST(addr == internet6_address::from_string("::2"));

    BOOST_TEST(++internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff") == internet6_address{});
}

BOOST_AUTO_TEST_CASE(decr)
{
    internet6_address addr = internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");

    --addr;
    BOOST_TEST(addr == internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffe"));

    addr--;
    BOOST_TEST(addr == internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffd"));

    BOOST_TEST(--internet6_address{} == internet6_address::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
}

BOOST_AUTO_TEST_CASE(add)
{
    internet6_address addr;

    addr += 0x10000;
    BOOST_TEST(addr == internet6_address::from_string("::1:0"));
    BOOST_TEST(addr + 0xff == internet6_address::from_string("::1:ff"));
}

BOOST_AUTO_TEST_CASE(sub)
{
    internet6_address addr = internet6_address::from_string("::10:0");

    addr -= 0x10000;
    BOOST_TEST(addr == internet6_address::from_string("::f:0"));
    BOOST_TEST(addr - 0xff == internet6_address::from_string("::e:ff01"));
}

BOOST_AUTO_TEST_CASE(compare)
{
    internet6_address addr1 = internet6_address::from_string("fe80::1");
    internet6_address addr2 = internet6_address::from_string("fe80::2");
    BOOST_TEST(addr1 == addr1);
    BOOST_TEST(addr1 != addr2);
    BOOST_TEST(addr1 < addr2);
    BOOST_TEST(!(addr1 >= addr2));
}

BOOST_AUTO_TEST_CASE(boost_asio_compatible)
{
    BOOST_TEST(internet6_address::from_string("ff::") == address_v6::from_string("ff::"));
    BOOST_TEST(internet6_address::from_string("ff::").to_bytes() == address_v6::from_string("ff::").to_bytes());
    BOOST_TEST(internet6_address::from_string("ff::").to_string() == address_v6::from_string("ff::").to_string());
}

BOOST_AUTO_TEST_CASE(string)
{
    BOOST_TEST(internet6_address{}.to_string() == "::");
    BOOST_TEST(internet6_address::from_string("fe80::1").to_string() == "fe80::1");

    error_code ec;
    BOOST_TEST(internet6_address::from_string("", ec) == internet6_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet6_address::from_string("::1.1", ec) == internet6_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet6_address::from_string(" fe80::", ec) == internet6_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet6_address::from_string("fe80::1:fffff", ec) == internet6_address::any());
    BOOST_TEST(ec != error_code());
}

BOOST_AUTO_TEST_SUITE_END()
