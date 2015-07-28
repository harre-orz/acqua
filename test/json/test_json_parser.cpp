#include <acqua/json/feed_parser.hpp>
#include <acqua/json/adapted_boost_ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

int main(int, char **)
{

    boost::property_tree::ptree pt;
    acqua::json::feed_parser<char, decltype(pt)> feed(pt);
    std::cin >> feed;
    boost::property_tree::write_json(std::cout, pt, true);

}
