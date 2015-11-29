#include <acqua/container/timed_lru_set.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(lru_set)

BOOST_AUTO_TEST_CASE(basic)
{
    acqua::container::timed_lru_set<int> x;
    x.max_size(10);
    for(int i = 0; i < 20; ++i)
        x.push(i);
    int i = 20;
    for(auto & e : x)
        BOOST_TEST(e == --i);
}

BOOST_AUTO_TEST_SUITE_END()
