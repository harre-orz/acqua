#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/sequenced_map.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(sequenced_map)

BOOST_AUTO_TEST_CASE(sequenced_map_basic)
{
    acqua::container::sequenced_map<int, int> map;
    map[2] = 1;
    map[1] = 2;
    map[3] = 3;
    BOOST_CHECK_EQUAL(map[2], 1);
    BOOST_CHECK_EQUAL(map[1], 2);
    BOOST_CHECK_EQUAL(map[3], 3);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+0)->second, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 2);
    BOOST_CHECK_EQUAL((map.begin()+2)->first, 3);
    BOOST_CHECK_EQUAL((map.begin()+2)->second, 3);
}

BOOST_AUTO_TEST_CASE(sequenced_multimap_basic)
{
    acqua::container::sequenced_multimap<int, int> map;
    map.insert(std::make_pair(2, 1));
    map.insert(std::make_pair(1, 2));
    map.insert(std::make_pair(3, 3));
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+0)->second, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 2);
    BOOST_CHECK_EQUAL((map.begin()+2)->first, 3);
    BOOST_CHECK_EQUAL((map.begin()+2)->second, 3);
}

BOOST_AUTO_TEST_SUITE_END()
