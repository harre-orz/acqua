#include <acqua/text/email_feed_parser.hpp>

int main()
{
    acqua::text::email_message<std::string> email;
    acqua::text::email_feed_parser<std::string> p(email);
    std::cin >> p;
    email.header.dump(std::cout);
}
