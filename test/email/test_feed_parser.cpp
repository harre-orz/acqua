#define BOOST_TEST_MAIN
#define CONSOLE_DEBUG
#include <boost/test/included/unit_test.hpp>

#include <acqua/email/feed_parser.hpp>
#include <acqua/email/basic_message.hpp>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(feed_parser)
{
    acqua::email::basic_message<std::string> email;
    acqua::email::feed_parser<decltype(email)> feed(email);
    std::cin >> feed;
    email.dump(std::cout);

}

BOOST_AUTO_TEST_SUITE_END()
