#define BOOST_TEST_MAIN    // main関数を定義

#include <acqua/email/basic_email.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE(email)

BOOST_AUTO_TEST_CASE(basics)
{

    acqua::email::basic_email<std::string> email;
    email.dump(std::cout);

    //email["Content-Type"] = "text/plain";
    //acqua::email::basic_email<std::string>::const_recursive_iterator it(email);
    //++it;
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
