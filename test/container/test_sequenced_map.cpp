#include <acqua/container/sequenced_map.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(sequenced_map)

BOOST_AUTO_TEST_CASE(basics)
{
    acqua::container::sequenced_map<int, int> map;
    map[2] = 1;
    map[1] = 2;
    map[3] = 3;
    BOOST_TEST(map[2] == 1);
    BOOST_TEST(map[1] == 2);
    BOOST_TEST(map[3] == 3);
    BOOST_TEST((map.begin()+0)->first == 2);
    BOOST_TEST((map.begin()+0)->second == 1);
    BOOST_TEST((map.begin()+1)->first == 1);
    BOOST_TEST((map.begin()+1)->second == 2);
    BOOST_TEST((map.begin()+2)->first == 3);
    BOOST_TEST((map.begin()+2)->second == 3);
    BOOST_TEST(map.size() == 3);
    BOOST_TEST(!map.empty());
    map.clear();
    BOOST_TEST(map.size() == 0);
    BOOST_TEST(map.empty());
}

BOOST_AUTO_TEST_CASE(insert)
{
    acqua::container::sequenced_map<int, int> map;

    auto e = map.insert(std::make_pair(1, 1));
    BOOST_TEST((map.begin() == e.first));
    BOOST_TEST(e.second);
    BOOST_TEST((map.begin()+0)->first == 1);

    e = map.insert(std::make_pair(2, 2));
    BOOST_TEST(((map.begin()+1) == e.first));
    BOOST_TEST(e.second);
    BOOST_TEST((map.begin()+1)->first == 2);

    e = map.insert(std::make_pair(1, 3));
    BOOST_TEST((map.begin() == e.first));
    BOOST_TEST(!e.second);
    BOOST_TEST((map.begin()+0)->first);
    BOOST_TEST(map.size() == 2);

    map.clear();
    BOOST_TEST(map.size() == 0);
}

BOOST_AUTO_TEST_CASE(emplace)
{
    acqua::container::sequenced_map<int, int> map;

    auto e = map.emplace(1, 1);
    BOOST_TEST((map.begin() == e.first));
    BOOST_TEST(e.second);
    BOOST_TEST((map.begin()+0)->first == 1);

    e = map.emplace(2, 2);
    BOOST_TEST(((map.begin()+1) == e.first));
    BOOST_TEST(e.second);
    BOOST_TEST((map.begin()+1)->first == 2);

    e = map.emplace(std::make_pair(1, 3));
    BOOST_TEST((map.begin() == e.first));
    BOOST_TEST(!e.second);
    BOOST_TEST((map.begin()+0)->first == 1);
    BOOST_TEST(map.size() == 2);

    map.clear();
    BOOST_TEST(map.size() == 0);
}

BOOST_AUTO_TEST_CASE(find)
{
    acqua::container::sequenced_map<int, int> map;
    for(int i = 1; i <= 3; ++i)
        map.emplace(i, i);

    auto it = map.find(1);
    BOOST_TEST((map.begin()+0 == it));
    it = map.find(2);
    BOOST_TEST((map.begin()+1 == it));
    it = map.find(3);
    BOOST_TEST((map.begin()+2 == it));
    it = map.find(4);
    BOOST_TEST((map.end() == it));
    it = map.find(-1);
    BOOST_TEST((map.end() == it));
}

BOOST_AUTO_TEST_CASE(erase)
{
    acqua::container::sequenced_map<int, int> map;
    for(int i = 0; i < 100; ++i)
        map.emplace(i, i);

    BOOST_TEST(map.size() == 100);
    auto it = map.find(50);
    BOOST_TEST(it->first == 50);
    it = map.erase(it);
    BOOST_TEST(it->first == 51);
    BOOST_TEST(map.size() == 99);
    it = map.erase(it, map.end());
    BOOST_TEST((it == map.end()));
    BOOST_TEST(map.size() == 50);
    it = map.erase(map.begin(), it);
    BOOST_TEST((it == map.begin()));
}

BOOST_AUTO_TEST_SUITE_END()
