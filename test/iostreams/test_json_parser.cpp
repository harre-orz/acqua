#include <acqua/iostreams/json_parser.hpp>
#include <acqua/iostreams/json_adapt_boost_ptree.hpp>
#include <boost/test/included/unit_test.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

BOOST_AUTO_TEST_SUITE(json_parser)

BOOST_AUTO_TEST_CASE(json_adapt_boost_ptree)
{
    boost::property_tree::ptree json1, json2;
    acqua::iostreams::json_parser<boost::property_tree::ptree> json(json1);
    boost::iostreams::stream<decltype(json)> out(json);
    std::ifstream ifs("sample.json");
    boost::iostreams::copy(ifs, out);
    boost::property_tree::read_json("sample.json", json2);
    BOOST_TEST((json1 == json2));
}

BOOST_AUTO_TEST_SUITE_END()
