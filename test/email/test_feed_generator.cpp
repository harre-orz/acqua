#define BOOST_TEST_MAIN
#define CONSOLE_DEBUG
#include <boost/test/included/unit_test.hpp>

#include <acqua/email/detail/feed_generator.hpp>
#include <acqua/email/detail/impl/feed_generator_impl.ipp>
#include <acqua/email/basic_message.hpp>
#include <acqua/email/utils/ascii_encoder.hpp>
#include <boost/format.hpp>
#include <sstream>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(feed_parser)
{
    std::stringstream ss;
    acqua::email::basic_message<std::string> email;
    //email["Content-Type"] = "application/octet-stream";
    email["Content-Type"]["charset"] = "ISO-2022-JP";
    email["Content-Transfer-Encoding"] = "quoted-printable";
    email["Content-Transfer-Encoding"] = "base64";
    email["Content-Disposition"] = "attach";
    email["Content-Disposition"]["filename"] = "あいうえお.txt";
    //email["Subject"] = "hello world 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0";
    email["Subject"] = "へおへおああああああああああああああああああああああああああああああああああ";
    std::ostream os(email);
    os << "test mail";
    os << "test mail2" << std::endl;
    os << "new line" << std::endl;
    os << "にごんご" << std::endl;
    os << "にごんご" << std::endl;
    os << "にごんご" << std::endl;
    os << "にごんご" << std::endl;
    os << "にごんご" << std::endl;
    os << "にごんご" << std::endl;

    //email.dump(std::cout);
    std::cout << "##############################" << std::endl;
    acqua::email::detail::feed_generator<decltype(email)> feed(email);
    ss << feed;

    std::cout << "##############################" << std::endl;
    std::cout << ss.str();
}

BOOST_AUTO_TEST_SUITE_END()
