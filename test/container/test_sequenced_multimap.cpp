#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/sequenced_map.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(sequenced_map)

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

    BOOST_CHECK_EQUAL(map.size(), 3);
    BOOST_CHECK_EQUAL(map.empty(), false);
    map.clear();
    BOOST_CHECK_EQUAL(map.size(), 0);
    BOOST_CHECK_EQUAL(map.empty(), true);
}

BOOST_AUTO_TEST_CASE(sequenced_multimap_insert)
{
    acqua::container::sequenced_multimap<int, int> map;

    auto e = map.insert(std::make_pair(1, 1));
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+0)->second, 1);

    e = map.insert(std::make_pair(2, 2));
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 2);

    e = map.insert(std::make_pair(1, 3));
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 3);
    BOOST_CHECK_EQUAL((map.begin()+2)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+2)->second, 2);
}

BOOST_AUTO_TEST_CASE(sequenced_multimap_emplace)
{
    acqua::container::sequenced_multimap<int, int> map;

    auto e = map.emplace(1, 1);
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+0)->second, 1);

    e = map.emplace(2, 2);
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 2);

    e = map.emplace(std::make_pair(1, 3));
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 1);
    BOOST_CHECK_EQUAL((map.begin()+1)->second, 3);
    BOOST_CHECK_EQUAL((map.begin()+2)->first, 2);
    BOOST_CHECK_EQUAL((map.begin()+2)->second, 2);
}

BOOST_AUTO_TEST_SUITE_END()
