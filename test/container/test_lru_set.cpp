#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/detail/lru_cache.hpp>
#include <acqua/container/lru_set.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(lru_set)

BOOST_AUTO_TEST_CASE(lru_set_basic)
{
    acqua::container::lru_set<int> x(128);
    x.reserve(10);
    for(int i = 0; i < 20; ++i) {
        x.push(i);
    }

    for(auto & e : x) {
        std::cout << e << std::endl;
    }
}

BOOST_AUTO_TEST_SUITE_END()
