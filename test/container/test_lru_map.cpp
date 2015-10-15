#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/container/lru_map.hpp>
#include <vector>
#include <utility>
#include <tuple>

BOOST_AUTO_TEST_SUITE(lru_map)


using lru_map = acqua::container::lru_map<int, int>;

BOOST_AUTO_TEST_CASE(lru_map_basics)
{
    const int elem_count = 10000;
    int i;

    lru_map map;
    BOOST_CHECK(map.empty() == true);
    BOOST_CHECK(map.size() == 0);
    BOOST_CHECK_EQUAL(map.begin() == map.end(), true);
    BOOST_CHECK_EQUAL(map.begin() != map.end(), false);
    BOOST_CHECK_EQUAL(map.rbegin() == map.rend(), true);
    BOOST_CHECK_EQUAL(map.rbegin() != map.rend(), false);

    for(i = 0; i < elem_count;) {
        map.push(std::make_pair(i, i * 2));
        BOOST_CHECK(map.empty() == false);
        ++i;
        BOOST_CHECK((int)map.size() == i);
    }

    i = elem_count;
    for(auto const & e : map) {
        --i;
        BOOST_CHECK_EQUAL(e.first, i);
        BOOST_CHECK_EQUAL(e.second, i * 2);
    }

    lru_map::iterator it;
    for(i = 0; i < elem_count; ++i) {
        it = map.find(i);
        BOOST_CHECK(it != map.end());
        BOOST_CHECK_EQUAL(it->first, i);
        BOOST_CHECK_EQUAL(it->second, i * 2);
    }

    BOOST_CHECK(map.size() == elem_count);
    map.max_size(1);
    BOOST_CHECK(map.size() == 1);
}

BOOST_AUTO_TEST_CASE(lru_map_insert)
{
    lru_map map;
    lru_map::iterator it;

    std::tie(it, std::ignore) = map.insert(std::make_pair(1, 2));
    BOOST_CHECK_EQUAL(it == map.begin(), true);
    BOOST_CHECK_EQUAL(++it == map.end(), true);
    it = map.insert(it, std::make_pair(1, 2));
    BOOST_CHECK_EQUAL(it == map.begin(), true);
    BOOST_CHECK_EQUAL(it != map.end(), true);
}

BOOST_AUTO_TEST_CASE(lru_map_emplace)
{
    lru_map map;
    lru_map::iterator it;
    bool res;

    // value &&
    std::tie(it, res) = map.emplace(1, 2);
    BOOST_CHECK_EQUAL(it == map.begin(), true);
    BOOST_CHECK(res == true);
    BOOST_CHECK(it->second == 2);

    // value const &
    const std::pair<int, int> val(2, 4);
    std::tie(it, res) = map.emplace(val);
    BOOST_CHECK_EQUAL(it == map.begin(), true);
    BOOST_CHECK(res == true);
    BOOST_CHECK(it->second == 4);
}

BOOST_AUTO_TEST_SUITE_END()
