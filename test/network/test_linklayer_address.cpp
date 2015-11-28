#include <acqua/network/linklayer_address.hpp>
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(linklayer_address)

BOOST_AUTO_TEST_CASE(linklayer_address__construct)
{
    using acqua::network::linklayer_address;

    BOOST_TEST(linklayer_address() == linklayer_address::any());
}

BOOST_AUTO_TEST_CASE(linklayer_address__increment)
{
    using acqua::network::linklayer_address;

    linklayer_address addr;

    ++addr;
    BOOST_TEST(addr == linklayer_address::from_string("00:00:00:00:00:01"));

    addr++;
    BOOST_TEST(addr == linklayer_address::from_string("00:00:00:00:00:02"));

    BOOST_TEST(++linklayer_address::broadcast() == linklayer_address());
}

BOOST_AUTO_TEST_CASE(linklayer_address__decrement)
{
    using acqua::network::linklayer_address;

    linklayer_address addr = linklayer_address::broadcast();

    --addr;
    BOOST_TEST(addr == linklayer_address::from_string("FF:FF:FF:FF:FF:FE"));

    addr--;
    BOOST_TEST(addr == linklayer_address::from_string("FF:FF:FF:FF:FF:FD"));

    BOOST_TEST(--linklayer_address::any() == linklayer_address::broadcast());
}

BOOST_AUTO_TEST_CASE(linklayer_address__add)
{
    using acqua::network::linklayer_address;

    linklayer_address addr;

    addr += 0x10;
    BOOST_TEST(addr == linklayer_address::from_string("00-00-00-00-00-10"));

    BOOST_TEST(addr + 0x10 == linklayer_address::from_string("00-00-00-00-00-20"));

    BOOST_TEST(linklayer_address() + 0x100 == linklayer_address::from_string("00-00-00-00-01-00"));
}

BOOST_AUTO_TEST_CASE(linklayer_address__sub)
{
    using acqua::network::linklayer_address;

    linklayer_address addr = linklayer_address::broadcast();

    addr -= 0x10;
    BOOST_TEST(addr == linklayer_address::from_string("ff-ff-ff-ff-ff-ef"));

    BOOST_TEST(addr - 0x10 == linklayer_address::from_string("ff-ff-ff-ff-ff-df"));

    BOOST_TEST(linklayer_address::broadcast() - 0x100 == linklayer_address::from_string("ff-ff-ff-ff-fe-ff"));
}


BOOST_AUTO_TEST_SUITE_END()
