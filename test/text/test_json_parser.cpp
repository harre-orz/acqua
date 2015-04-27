#include <acqua/text/json_feed_parser.hpp>
#include <acqua/text/adapt/boost_ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

int main(int, char **)
{

    boost::property_tree::ptree pt;
    acqua::text::json_feed_parser<char, decltype(pt)> feed(pt);

    char ch;
    while (feed.good()) {
        std::cin >> ch;
        std::cout << ch;
        feed << ch;
    }

    std::cout << std::endl << std::endl;
    boost::property_tree::write_json(std::cout, pt, true);
}
