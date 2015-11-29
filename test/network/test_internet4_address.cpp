#include <acqua/network/internet4_address.hpp>
#include <boost/test/included/unit_test.hpp>
#include <random>
#include <vector>

BOOST_AUTO_TEST_SUITE(internet4_address)

using acqua::network::internet4_address;
using boost::asio::ip::address_v4;
using boost::system::error_code;

std::random_device rand;

BOOST_AUTO_TEST_CASE(construct)
{
    BOOST_TEST(internet4_address{} == internet4_address::any());
    BOOST_TEST(internet4_address(192,168,0,1) == internet4_address::from_string("192.168.0.1"));
}

BOOST_AUTO_TEST_CASE(loopback)
{
    internet4_address addr_1{0x7F000001};
    BOOST_TEST(addr_1 == internet4_address::loopback());
    BOOST_TEST(addr_1 == internet4_address::from_string("127.0.0.1"));
    BOOST_TEST(addr_1.is_loopback());
    BOOST_TEST(addr_1.is_class_a());
    BOOST_TEST(!addr_1.is_class_b());
    BOOST_TEST(!addr_1.is_class_c());
}

BOOST_AUTO_TEST_CASE(class_a)
{
    internet4_address addr_1{10,0,0,1};
    BOOST_TEST(addr_1 == internet4_address::from_string("10.0.0.1"));
    BOOST_TEST(!addr_1.is_loopback());
    BOOST_TEST(addr_1.is_class_a());
    BOOST_TEST(!addr_1.is_class_b());
    BOOST_TEST(!addr_1.is_class_c());
}

BOOST_AUTO_TEST_CASE(class_b)
{
    internet4_address addr_1{172,16,0,1};
    BOOST_TEST(addr_1 == internet4_address::from_string("172.16.0.1"));
    BOOST_TEST(!addr_1.is_loopback());
    BOOST_TEST(!addr_1.is_class_a());
    BOOST_TEST(addr_1.is_class_b());
    BOOST_TEST(!addr_1.is_class_c());
}

BOOST_AUTO_TEST_CASE(class_c)
{
    internet4_address addr_1{192,168,0,1};
    BOOST_TEST(addr_1 == internet4_address::from_string("192.168.0.1"));
    BOOST_TEST(!addr_1.is_loopback());
    BOOST_TEST(!addr_1.is_class_a());
    BOOST_TEST(!addr_1.is_class_b());
    BOOST_TEST(addr_1.is_class_c());
}

BOOST_AUTO_TEST_CASE(incr)
{
    internet4_address addr;

    ++addr;
    BOOST_TEST(addr == internet4_address::from_string("0.0.0.1"));

    addr++;
    BOOST_TEST(addr == internet4_address::from_string("0.0.0.2"));

    BOOST_TEST(++internet4_address::broadcast() == internet4_address());
}

BOOST_AUTO_TEST_CASE(incr_random)
{
    internet4_address addr;
    auto n = addr.to_ulong();
    for(int i = 0; i < 1000; ++i) {
        BOOST_TEST(addr == internet4_address(n));
        std::size_t j = (rand() % 100);
        while(j--) {
            ++addr;
            ++n;
        }
    }
}

BOOST_AUTO_TEST_CASE(decr)
{
    internet4_address addr = internet4_address::broadcast();

    --addr;
    BOOST_TEST(addr == internet4_address::from_string("255.255.255.254"));

    addr--;
    BOOST_TEST(addr == internet4_address::from_string("255.255.255.253"));

    BOOST_TEST(--internet4_address() == internet4_address::broadcast());
}

BOOST_AUTO_TEST_CASE(decr_random)
{
    internet4_address addr = internet4_address::broadcast();
    auto n = addr.to_ulong();
    for(int i = 0; i < 1000; ++i) {
        BOOST_TEST(addr == internet4_address(n));
        std::size_t j = (rand() % 100);
        while(j--) {
            --addr;
            --n;
        }
    }
}

BOOST_AUTO_TEST_CASE(add)
{
    internet4_address addr;

    addr += 256;
    BOOST_TEST(addr == internet4_address::from_string("0.0.1.0"));

    BOOST_TEST(addr + 256 == internet4_address::from_string("0.0.2.0"));
}

BOOST_AUTO_TEST_CASE(add_random)
{
    internet4_address addr;
    auto n = addr.to_ulong();
    for(int i = 0; i < 1000; ++i) {
        BOOST_TEST(addr == internet4_address(n));
        std::ptrdiff_t j = (rand() % 1000);
        addr += j;
        n += j;
    }
}

BOOST_AUTO_TEST_CASE(sub)
{
    internet4_address addr = internet4_address::broadcast();

    addr -= 256;
    BOOST_TEST(addr == internet4_address::from_string("255.255.254.255"));

    BOOST_TEST(addr - 256 == internet4_address::from_string("255.255.253.255"));
}

BOOST_AUTO_TEST_CASE(sub_random)
{
    internet4_address addr = internet4_address::broadcast();
    auto n = addr.to_ulong();
    for(int i = 0; i < 1000; ++i) {
        BOOST_TEST(addr == internet4_address(n));
        std::ptrdiff_t j = (rand() % 1000);
        addr -= j;
        n -= j;
    }
}

BOOST_AUTO_TEST_CASE(compare)
{
    internet4_address addr1 = internet4_address::from_string("192.168.0.1");
    internet4_address addr2 = internet4_address::from_string("192.168.0.2");
    BOOST_TEST(addr1 == addr1);
    BOOST_TEST(addr1 != addr2);
    BOOST_TEST(addr1 < addr2);
    BOOST_TEST(!(addr1 >= addr2));
}

BOOST_AUTO_TEST_CASE(boost_asio_compatible)
{
    BOOST_TEST(internet4_address(1234) == address_v4(1234));
    BOOST_TEST(internet4_address::from_string("1.2.3.4").to_ulong() == address_v4::from_string("1.2.3.4").to_ulong());
    BOOST_TEST(internet4_address::from_string("1.2.3.4") == address_v4::from_string("1.2.3.4"));
    BOOST_TEST(internet4_address::from_string("1.2.3.4").to_bytes() == address_v4::from_string("1.2.3.4").to_bytes());
    BOOST_TEST(internet4_address::from_string("1.2.3.4").to_string() == address_v4::from_string("1.2.3.4").to_string());
}

BOOST_AUTO_TEST_CASE(string)
{
    BOOST_TEST(internet4_address().to_string() == "0.0.0.0");
    BOOST_TEST(internet4_address::from_string("1.2.3.4").to_string() == "1.2.3.4");

    error_code ec;
    BOOST_TEST(internet4_address::from_string("", ec) == internet4_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet4_address::from_string("0.0.0.0.0" , ec) == internet4_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet4_address::from_string(" 192.168.0.1", ec) == internet4_address::any());
    BOOST_TEST(ec != error_code());

    ec.clear();
    BOOST_TEST(internet4_address::from_string(" 192.168.0.256", ec) == internet4_address::any());
    BOOST_TEST(ec != error_code());
}

BOOST_AUTO_TEST_CASE(hash_func)
{
    int d1 = -1;
    internet4_address addr1;
    int d2 = -1;
    internet4_address addr2;
    int d3 = -1;
    (void) d1; (void) d2; (void) d3;

    BOOST_TEST(hash_value(addr1) == hash_value(addr2));
    for(int i = 0; i < 1000; ++i) {
        int n = rand() % 1000;
        addr1 += n;
        addr2 += n;
        BOOST_TEST(hash_value(addr1) == hash_value(addr2));
    }
}

BOOST_AUTO_TEST_CASE(netmask)
{
    std::vector<std::string> netmasks = {
        "0.0.0.0",
        "128.0.0.0",
        "192.0.0.0",
        "224.0.0.0",
        "240.0.0.0",
        "248.0.0.0",
        "252.0.0.0",
        "254.0.0.0",
        "255.0.0.0",
        "255.128.0.0",
        "255.192.0.0",
        "255.224.0.0",
        "255.240.0.0",
        "255.248.0.0",
        "255.252.0.0",
        "255.254.0.0",
        "255.255.0.0",
        "255.255.128.0",
        "255.255.192.0",
        "255.255.224.0",
        "255.255.240.0",
        "255.255.248.0",
        "255.255.252.0",
        "255.255.254.0",
        "255.255.255.0",
        "255.255.255.128",
        "255.255.255.192",
        "255.255.255.224",
        "255.255.255.240",
        "255.255.255.248",
        "255.255.255.252",
        "255.255.255.254",
        "255.255.255.255",
    };

    for(std::size_t i = 0; i < netmasks.size(); ++i) {
        auto addr = internet4_address::from_string(netmasks[i]);
        BOOST_TEST(netmask_length(addr) == i);
        if (addr.is_unspecified())
            BOOST_TEST(!addr.is_netmask());
        else
            BOOST_TEST(addr.is_netmask());
    }
}

BOOST_AUTO_TEST_SUITE_END()
