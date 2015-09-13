#define BOOST_TEST_MAIN
#define CONSOLE_DEBUG
#include <boost/test/included/unit_test.hpp>

#include <acqua/email/detail/feed_parser.hpp>
#include <acqua/email/detail/impl/feed_parser_impl.ipp>
#include <acqua/email/basic_message.hpp>
#include <boost/format.hpp>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(feed_parser)
{
    acqua::email::basic_message<std::string> email;
    acqua::email::detail::feed_parser<decltype(email)> feed(email);
    std::cin >> feed;

    int i = 0;
    for(auto it = email.recbegin(); !(it == email.recend()); ++it) {
        it->save_as( (boost::format("hoge%d.dat") % i++).str() );
    }

}

BOOST_AUTO_TEST_SUITE_END()
