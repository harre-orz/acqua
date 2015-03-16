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

    BOOST_CHECK_EQUAL(map.size(), 3);
    BOOST_CHECK_EQUAL(map.empty(), false);
    map.clear();
    BOOST_CHECK_EQUAL(map.size(), 0);
    BOOST_CHECK_EQUAL(map.empty(), true);
}

BOOST_AUTO_TEST_CASE(sequenced_map_insert)
{
    acqua::container::sequenced_map<int, int> map;

    auto e = map.insert(std::make_pair(1, 1));
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);

    e = map.insert(std::make_pair(2, 2));
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 2);

    e = map.insert(std::make_pair(1, 3));
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, false);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);

    BOOST_CHECK_EQUAL(map.size(), 2);
    map.clear();
    BOOST_CHECK_EQUAL(map.size(), 0);

}

BOOST_AUTO_TEST_CASE(sequenced_map_emplace)
{
    acqua::container::sequenced_map<int, int> map;

    auto e = map.emplace(1, 1);
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);

    e = map.emplace(2, 2);
    BOOST_CHECK(map.begin()+1 == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL((map.begin()+1)->first, 2);

    e = map.emplace(std::make_pair(1, 3));
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, false);
    BOOST_CHECK_EQUAL((map.begin()+0)->first, 1);

    BOOST_CHECK_EQUAL(map.size(), 2);
    map.clear();
    BOOST_CHECK_EQUAL(map.size(), 0);
}

BOOST_AUTO_TEST_CASE(sequenced_map_find)
{
    acqua::container::sequenced_map<int, int> map;
    for(int i = 1; i <= 3; ++i)
        map.emplace(i, i);

    auto it = map.find(1);
    BOOST_CHECK(map.begin()+0 == it);
    it = map.find(2);
    BOOST_CHECK(map.begin()+1 == it);
    it = map.find(3);
    BOOST_CHECK(map.begin()+2 == it);
    it = map.find(4);
    BOOST_CHECK(map.end() == it);
    it = map.find(-1);
    BOOST_CHECK(map.end() == it);
}

BOOST_AUTO_TEST_CASE(sequenced_map_erase)
{
    acqua::container::sequenced_map<int, int> map;
    for(int i = 0; i < 100; ++i)
        map.emplace(i, i);

    BOOST_CHECK_EQUAL(map.size(), 100);
    auto it = map.find(50);
    BOOST_CHECK_EQUAL(it->first, 50);
    it = map.erase(it);
    BOOST_CHECK_EQUAL(it->first, 51);
    BOOST_CHECK_EQUAL(map.size(), 99);
    it = map.erase(it, map.end());
    BOOST_CHECK(it == map.end());
    BOOST_CHECK_EQUAL(map.size(), 50);
    it = map.erase(map.begin(), it);
    BOOST_CHECK(it == map.begin());
}

BOOST_AUTO_TEST_SUITE_END()
