#include <acqua/container/timed_lru_map.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(lru_set)

using lru_map = acqua::container::timed_lru_map<int, int>;

BOOST_AUTO_TEST_CASE(basics)
{
    const int elem_count = 10000;
    int i;

    lru_map map;
    for(i = 0; i < elem_count; ++i) {
        map.push(std::make_pair(i, i * 2));
    }

    i = elem_count;
    for(auto const & e : map) {
        --i;
        BOOST_TEST(e.first == i);
        BOOST_TEST(e.second == i * 2);
    }

    decltype(map.begin()) it;
    for(i = 0; i < elem_count; ++i) {
        it = map.find(i);
        BOOST_TEST((it != map.end()));
        BOOST_TEST(it->first == i);
        BOOST_TEST(it->second == i * 2);
    }
}

BOOST_AUTO_TEST_SUITE_END()
