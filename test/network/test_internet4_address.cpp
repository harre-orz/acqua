#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/network/internet4_address.hpp>

BOOST_AUTO_TEST_SUITE(internet4_address)

using acqua::network::internet4_address;
using boost::asio::ip::address_v4;
using boost::system::error_code;

BOOST_AUTO_TEST_CASE(internet4_address__construct)
{
    BOOST_CHECK_EQUAL(internet4_address(), internet4_address::any());

    BOOST_CHECK_EQUAL(internet4_address({192,168,0,1}), internet4_address::from_string("192.168.0.1"));
}

BOOST_AUTO_TEST_CASE(internet4_address__loopback)
{
    internet4_address addr_1(0x7F000001);
    BOOST_CHECK_EQUAL(addr_1, internet4_address::loopback());
    BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("127.0.0.1"));
    BOOST_CHECK_EQUAL(addr_1.is_loopback(), true);
    BOOST_CHECK_EQUAL(addr_1.is_class_a(), true);
    BOOST_CHECK_EQUAL(addr_1.is_class_b(), false);
    BOOST_CHECK_EQUAL(addr_1.is_class_c(), false);
}

BOOST_AUTO_TEST_CASE(internet4_address__class_a)
{
    internet4_address addr_1({10,0,0,1});
    BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("10.0.0.1"));
    BOOST_CHECK_EQUAL(addr_1.is_class_a(), true);
}

BOOST_AUTO_TEST_CASE(internet4_address__class_b)
{
    internet4_address addr_1({172,16,0,1});
    BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("172.16.0.1"));
    BOOST_CHECK_EQUAL(addr_1.is_class_b(), true);
}

BOOST_AUTO_TEST_CASE(internet4_address__class_c)
{
    internet4_address addr_1({192,168,0,1});
    BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("192.168.0.1"));
    BOOST_CHECK_EQUAL(addr_1.is_class_c(), true);
}

BOOST_AUTO_TEST_CASE(internet4_address__increment)
{
    internet4_address addr;

    ++addr;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.0.1"));

    addr++;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.0.2"));

    BOOST_CHECK_EQUAL(++internet4_address::broadcast(), internet4_address());
}

BOOST_AUTO_TEST_CASE(internet4_address__decrement)
{
    internet4_address addr = internet4_address::broadcast();

    --addr;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.255.254"));

    addr--;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.255.253"));

    BOOST_CHECK_EQUAL(--internet4_address(), internet4_address::broadcast());
}

BOOST_AUTO_TEST_CASE(internet4_address__add)
{
    internet4_address addr;

    addr += 256;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.1.0"));

    BOOST_CHECK_EQUAL(addr + 256, internet4_address::from_string("0.0.2.0"));
}

BOOST_AUTO_TEST_CASE(internet4_address__sub)
{
    internet4_address addr = internet4_address::broadcast();

    addr -= 256;
    BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.254.255"));

    BOOST_CHECK_EQUAL(addr - 256, internet4_address::from_string("255.255.253.255"));
}

BOOST_AUTO_TEST_CASE(internet4_address__compare)
{
    internet4_address addr1 = internet4_address::from_string("192.168.0.1");
    internet4_address addr2 = internet4_address::from_string("192.168.0.2");
    BOOST_CHECK_EQUAL(addr1 == addr1, true);
    BOOST_CHECK_EQUAL(addr1 != addr2, true);
    BOOST_CHECK_EQUAL(addr1 < addr2, true);
    BOOST_CHECK_EQUAL(addr1 >= addr2, false);
}

BOOST_AUTO_TEST_CASE(internet4_address__boost_asio_compatible)
{
    BOOST_CHECK_EQUAL(internet4_address(1234), address_v4(1234));
    BOOST_CHECK_EQUAL(internet4_address::from_string("1.2.3.4").to_ulong(), address_v4::from_string("1.2.3.4").to_ulong());
    BOOST_CHECK_EQUAL(internet4_address::from_string("1.2.3.4") == address_v4::from_string("1.2.3.4"), true);
    BOOST_CHECK_EQUAL(internet4_address::from_string("1.2.3.4").to_bytes() == address_v4::from_string("1.2.3.4").to_bytes(), true);
    BOOST_CHECK_EQUAL(internet4_address::from_string("1.2.3.4").to_string() == address_v4::from_string("1.2.3.4").to_string(), true);
}

BOOST_AUTO_TEST_CASE(internet6_address__string)
{
    BOOST_CHECK_EQUAL(internet4_address().to_string(), "0.0.0.0");
    BOOST_CHECK_EQUAL(internet4_address::from_string("1.2.3.4").to_string(), "1.2.3.4");

    error_code ec;
    BOOST_CHECK_EQUAL(internet4_address::from_string("", ec), internet4_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet4_address::from_string("0.0.0.0.0" , ec), internet4_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet4_address::from_string(" 192.168.0.1", ec), internet4_address::any());
    BOOST_CHECK_NE(ec, error_code());

    ec.clear();
    BOOST_CHECK_EQUAL(internet4_address::from_string(" 192.168.0.256", ec), internet4_address::any());
    BOOST_CHECK_NE(ec, error_code());
}

BOOST_AUTO_TEST_SUITE_END()
