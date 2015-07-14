#define BOOST_TEST_MAIN    // main関数を定義

#include <acqua/email/basic_message.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(basic_message)
{

    acqua::email::basic_message<std::string> message;
    message.dump(std::cout);
    message["Content-Type"] = "text/plain";
    acqua::email::basic_message<std::string>::const_recursive_iterator it(message);
    ++it;
    // acqua::email::basic_message<char> message;
    // message["Content-Type"] = "Text/Plain";
    // message["Content-Transfer-Encoding"] = "7bit";
    // message["Content-Type"]["charset"] = "utf-8";
    // std::iostream ios(message);
    // ios << "hello";
    // std::cout << message.str();
    // ios << "hello";
    // message.dump(std::cout);
    // std::cout << "maintype:" << message.get_content_maintype() << std::endl;
    // std::cout << "subtype :" << message.get_content_subtype() << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
