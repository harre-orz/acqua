#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/detail/lru_cache.hpp>
#include <acqua/container/lru_set.hpp>
#include <acqua/container/lru_map.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(lru_set)

BOOST_AUTO_TEST_CASE(lru_set_basic)
{
    acqua::container::lru_map<std::string, int> x(128);
    x.max_size(10);
    for(int i = 0; i < 20; ++i) {
        auto v = std::make_pair(boost::lexical_cast<std::string>(i), i);
        x.push(std::move(v));
    }

    for(auto & e : x) {
        std::cout << e.first << std::endl;
    }
}

BOOST_AUTO_TEST_SUITE_END()
