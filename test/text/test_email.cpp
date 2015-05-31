#include <acqua/text/email_feed_parser.hpp>

int main()
{
    acqua::text::email_message<std::string> email;
    acqua::text::email_feed_parser<std::string> p(email);
    std::string text = "From: test@example.com\r\n\r\n.\r\n";

    p.parse(text.c_str(), text.size());
    email.header.dump(std::cout);
}
