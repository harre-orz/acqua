#include <acqua/text/json_feed_parser.hpp>
#include <acqua/text/adapt/boost_ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

int main(int, char ** argv)
{
    boost::property_tree::ptree pt;
    acqua::text::json_feed_parser<decltype(pt), char> feed(pt);
    feed << argv[1];

    std::cout << "---------" << std::endl;
    boost::property_tree::write_xml(std::cout, pt);
}
