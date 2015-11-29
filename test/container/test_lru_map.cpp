#include <acqua/container/lru_map.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>
#include <utility>
#include <tuple>

BOOST_AUTO_TEST_SUITE(lru_map)


using lru_map = acqua::container::lru_map<int, int>;

BOOST_AUTO_TEST_CASE(basics)
{
    const int elem_count = 10000;
    int i;

    lru_map map;
    BOOST_TEST(map.empty());
    BOOST_TEST(map.size() == 0);
    BOOST_TEST((map.begin() == map.end()));
    BOOST_TEST(!(map.begin() != map.end()));
    BOOST_TEST((map.rbegin() == map.rend()));
    BOOST_TEST(!(map.rbegin() != map.rend()));

    for(i = 0; i < elem_count;) {
        map.push(std::make_pair(i, i * 2));
        BOOST_TEST(!map.empty());
        ++i;
        BOOST_TEST(map.size() == static_cast<std::size_t>(i));
    }

    i = elem_count;
    for(auto const & e : map) {
        --i;
        BOOST_TEST(e.first == i);
        BOOST_TEST(e.second == i * 2);
    }

    lru_map::iterator it;
    for(i = 0; i < elem_count; ++i) {
        it = map.find(i);
        BOOST_TEST((it != map.end()));
        BOOST_TEST(it->first == i);
        BOOST_TEST(it->second == i * 2);
    }

    BOOST_TEST(map.size() == elem_count);
    map.max_size(1);
    BOOST_TEST(map.size() == 1);
}

BOOST_AUTO_TEST_CASE(insert)
{
    lru_map map;
    lru_map::iterator it;

    std::tie(it, std::ignore) = map.insert(std::make_pair(1, 2));
    BOOST_TEST((it == map.begin()));
    BOOST_TEST((++it == map.end()));

    it = map.insert(it, std::make_pair(1, 2));
    BOOST_TEST((it == map.begin()));
    BOOST_TEST((it != map.end()));
}

BOOST_AUTO_TEST_CASE(emplace)
{
    lru_map map;
    lru_map::iterator it;
    bool res;

    // value &&
    std::tie(it, res) = map.emplace(1, 2);
    BOOST_TEST((it == map.begin()));
    BOOST_TEST(res);
    BOOST_TEST(it->second == 2);

    // value const &
    const std::pair<int, int> val(2, 4);
    std::tie(it, res) = map.emplace(val);
    BOOST_TEST((it == map.begin()));
    BOOST_TEST(res);
    BOOST_TEST(it->second == 4);
}

BOOST_AUTO_TEST_SUITE_END()
