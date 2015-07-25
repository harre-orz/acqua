#include <acqua/text/json_feed_parser.hpp>
#include <acqua/text/adapted/boost_ptree_json.hpp>
#include <boost/property_tree/json_parser.hpp>

int main(int, char **)
{

    boost::property_tree::ptree pt;
    acqua::text::json_feed_parser<char, decltype(pt)> feed(pt);
    std::cin >> feed;
    boost::property_tree::write_json(std::cout, pt, true);
}
