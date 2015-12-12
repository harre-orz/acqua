#include <acqua/email/address.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(address)

BOOST_AUTO_TEST_CASE(construct)
{
    acqua::email::address addr;
    BOOST_TEST(addr.addrspec == "");
    BOOST_TEST(addr.namespec == "");
    BOOST_TEST((addr == acqua::email::address()));
}

BOOST_AUTO_TEST_CASE(make_address)
{
    acqua::email::address addr;
    addr = acqua::email::make_address("test@example.com");
    BOOST_TEST(addr.namespec == "");
    BOOST_TEST(addr.addrspec == "test@example.com");

    addr = acqua::email::make_address("<example@example.com>");
    BOOST_TEST(addr.namespec == "");
    BOOST_TEST(addr.addrspec == "example@example.com");

    addr = acqua::email::make_address(" foo bar < test@example.com > ");  // 空白はトリムされる
    BOOST_TEST(addr.namespec == "foo bar");
    BOOST_TEST(addr.addrspec == "test@example.com");

    addr = acqua::email::make_address(" hello \r\n <test@example.com>");  // 改行もトリムされる
    BOOST_TEST(addr.namespec == "hello");
    BOOST_TEST(addr.addrspec == "test@example.com");
}

BOOST_AUTO_TEST_CASE(parse_to_addresses)
{
    std::vector<acqua::email::address> addrs;
    std::string text = "foo@example.com, c++ <bar@example.co.jp>";
    auto cnt = acqua::email::parse_to_addresses(text.begin(), text.end(), addrs);
    BOOST_TEST(cnt == 2);
    BOOST_TEST(addrs[0].namespec == "");
    BOOST_TEST(addrs[0].addrspec == "foo@example.com");
    BOOST_TEST(addrs[1].namespec == "c++");
    BOOST_TEST(addrs[1].addrspec == "bar@example.co.jp");

    text = "hoge fuga piyo <foo@example.com>,";  // 最後に , があっても無視される
    cnt += acqua::email::parse_to_addresses(text.begin(), text.end(), addrs);
    BOOST_TEST(cnt == 3);
    BOOST_TEST(addrs[2].namespec == "hoge fuga piyo");
    BOOST_TEST(addrs[2].addrspec == "foo@example.com");

    text = ",hoge fuga piyo <foo@example.com>";  // 最初に , があっても無視される
    cnt += acqua::email::parse_to_addresses(text.begin(), text.end(), addrs);
    BOOST_TEST(cnt == 4);
    BOOST_TEST(addrs[3].namespec == "hoge fuga piyo");
    BOOST_TEST(addrs[3].addrspec == "foo@example.com");
}

BOOST_AUTO_TEST_SUITE_END()
