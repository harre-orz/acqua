#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/sequenced_map.hpp>

BOOST_AUTO_TEST_SUITE(sequenced_map)

BOOST_AUTO_TEST_CASE(sequenced_multimap_basic)
{
    acqua::container::sequenced_multimap<int, int> map;
    map.insert(std::make_pair(2, 1));
    map.insert(std::make_pair(1, 2));
    map.insert(std::make_pair(3, 3));
    auto it = map.begin();
    BOOST_CHECK_EQUAL(it->first, 2);
    BOOST_CHECK_EQUAL(it->second, 1);
    ++it;
    BOOST_CHECK_EQUAL(it->first, 1);
    BOOST_CHECK_EQUAL(it->second, 2);
    ++it;
    BOOST_CHECK_EQUAL(it->first, 3);
    BOOST_CHECK_EQUAL(it->second, 3);

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
    auto it = map.begin();
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 1);
    BOOST_CHECK_EQUAL(it->second, 1);

    e = map.insert(std::make_pair(2, 2));
    it = ++map.begin();
    BOOST_CHECK(it == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 2);
    BOOST_CHECK_EQUAL(it->second, 2);
    
    e = map.insert(std::make_pair(1, 3));
    it = ++map.begin();
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 1);
    BOOST_CHECK_EQUAL(it->second, 1);
    ++it;
    BOOST_CHECK_EQUAL(it->first, 2);
    BOOST_CHECK_EQUAL(it->second, 2);
}

BOOST_AUTO_TEST_CASE(sequenced_multimap_emplace)
{
    acqua::container::sequenced_multimap<int, int> map;

    auto e = map.emplace(1, 1);
    auto it = map.begin();
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 1);
    BOOST_CHECK_EQUAL(it->second, 1);

    e = map.emplace(2, 2);
    it = ++map.begin();
    BOOST_CHECK(it == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 2);
    BOOST_CHECK_EQUAL(it->second, 2);

    e = map.emplace(std::make_pair(1, 3));
    it = ++map.begin();
    BOOST_CHECK(map.begin() == e.first);
    BOOST_CHECK_EQUAL(e.second, true);
    BOOST_CHECK_EQUAL(it->first, 1);
    BOOST_CHECK_EQUAL(it->second, 1);
    ++it;
    BOOST_CHECK_EQUAL(it->first, 2);
    BOOST_CHECK_EQUAL(it->second, 2);
}

BOOST_AUTO_TEST_SUITE_END()
