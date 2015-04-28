#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define private public
#define protected public
#include <acqua/network/internet4_prefix.hpp>

BOOST_AUTO_TEST_SUITE(internet4_prefix)

BOOST_AUTO_TEST_CASE(internet4_prefix__construct)
{
    using acqua::network::internet4_prefix;
    internet4_prefix x;
    std::cout << x;
}

// BOOST_AUTO_TEST_CASE(internet4_address__loopback)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr_1({127,0,0,1});
//     BOOST_CHECK_EQUAL(addr_1, internet4_address::loopback());
//     BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("127.0.0.1"));
//     BOOST_CHECK_EQUAL(addr_1.is_loopback(), true);
//     BOOST_CHECK_EQUAL(addr_1.is_class_a(), true);
//     BOOST_CHECK_EQUAL(addr_1.is_class_b(), false);
//     BOOST_CHECK_EQUAL(addr_1.is_class_c(), false);
// }

// BOOST_AUTO_TEST_CASE(internet4_address__class_a)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr_1({10,0,0,1});
//     BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("10.0.0.1"));
//     BOOST_CHECK_EQUAL(addr_1.is_class_a(), true);
// }

// BOOST_AUTO_TEST_CASE(internet4_address__class_b)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr_1({172,16,0,1});
//     BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("172.16.0.1"));
//     BOOST_CHECK_EQUAL(addr_1.is_class_b(), true);
// }

// BOOST_AUTO_TEST_CASE(internet4_address__class_c)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr_1({192,168,0,1});
//     BOOST_CHECK_EQUAL(addr_1, internet4_address::from_string("192.168.0.1"));
//     BOOST_CHECK_EQUAL(addr_1.is_class_c(), true);
// }

// BOOST_AUTO_TEST_CASE(internet4_address__increment)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr;

//     ++addr;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.0.1"));

//     addr++;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.0.2"));

//     BOOST_CHECK_EQUAL(++internet4_address::broadcast(), internet4_address());
// }

// BOOST_AUTO_TEST_CASE(internet4_address__decrement)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr = internet4_address::broadcast();

//     --addr;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.255.254"));

//     addr--;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.255.253"));

//     BOOST_CHECK_EQUAL(--internet4_address(), internet4_address::broadcast());
// }

// BOOST_AUTO_TEST_CASE(internet4_address__add)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr;

//     addr += 256;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("0.0.1.0"));

//     BOOST_CHECK_EQUAL(addr + 256, internet4_address::from_string("0.0.2.0"));
// }

// BOOST_AUTO_TEST_CASE(internet4_address__sub)
// {
//     using acqua::network::internet4_address;

//     internet4_address addr = internet4_address::broadcast();

//     addr -= 256;
//     BOOST_CHECK_EQUAL(addr, internet4_address::from_string("255.255.254.255"));

//     BOOST_CHECK_EQUAL(addr - 256, internet4_address::from_string("255.255.253.255"));
// }

BOOST_AUTO_TEST_SUITE_END()
