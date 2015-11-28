#include <acqua/iostreams/json_parser.hpp>
#include <acqua/iostreams/json_adapt_boost_ptree.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>

int main()
{
    boost::property_tree::ptree json;
    acqua::iostreams::json_parser<boost::property_tree::ptree> parser(json);
    char buf[] = "[1,2,3,4,5]";
    parser.write(buf, strlen(buf));
    boost::property_tree::write_json(std::cout, json);
}

