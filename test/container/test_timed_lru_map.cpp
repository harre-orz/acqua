#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/container/timed_lru_map.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(lru_set)

using lru_map = acqua::container::timed_lru_map<int, int>;

BOOST_AUTO_TEST_CASE(lru_set_basics)
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
        BOOST_CHECK_EQUAL(e.first, i);
        BOOST_CHECK_EQUAL(e.second, i * 2);
    }

    i = 0;
    decltype(map.begin()) it;
    for(int i = 0; i < elem_count; ++i) {
        it = map.find(i);
        BOOST_CHECK(it != map.end());
        BOOST_CHECK_EQUAL(it->first, i);
        BOOST_CHECK_EQUAL(it->second, i * 2);
    }
}

BOOST_AUTO_TEST_SUITE_END()
