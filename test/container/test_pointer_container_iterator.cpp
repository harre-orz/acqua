#include <acqua/container/shared_container_iterator.hpp>
#include <acqua/container/unique_container_iterator.hpp>
#include <boost/test/included/unit_test.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(pointer_container_iterator)

BOOST_AUTO_TEST_CASE(shared_container_iterator)
{
    acqua::container::shared_container_iterator< std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_TEST((end == end));
    BOOST_TEST((it != end));
    BOOST_TEST((end != it));
    BOOST_TEST((*it == 1));
    BOOST_TEST((++it != end));
    BOOST_TEST((*it == 2));
    BOOST_TEST((it++ != end));
    BOOST_TEST((*it++ == 3));
    BOOST_TEST((end != ++it));
    BOOST_TEST((end != it++));
    BOOST_TEST((it == end));
    BOOST_TEST((it.get() != nullptr));
    BOOST_TEST((end.get() == nullptr));
}

BOOST_AUTO_TEST_CASE(shared_container_const_iterator)
{
    acqua::container::shared_container_iterator<const std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_TEST((end == end));
    BOOST_TEST((it != end));
    BOOST_TEST((end != it));
    BOOST_TEST((*it == 1));
    BOOST_TEST((++it != end));
    BOOST_TEST((*it == 2));
    BOOST_TEST((it++ != end));
    BOOST_TEST((*it++ == 3));
    BOOST_TEST((end != ++it));
    BOOST_TEST((end != it++));
    BOOST_TEST((it == end));
    BOOST_TEST((it.get() != nullptr));
    BOOST_TEST((end.get() == nullptr));
}

BOOST_AUTO_TEST_CASE(unique_container_iterator_pointer)
{
    acqua::container::unique_container_iterator< std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_TEST((end == end));
    BOOST_TEST((it != end));
    BOOST_TEST((end != it));
    BOOST_TEST((*it == 1));
    BOOST_TEST((++it != end));
    BOOST_TEST((*it == 2));
    BOOST_TEST((++it != end));
    BOOST_TEST((*it == 3));
    ++it;
    BOOST_TEST((end != ++it));
    *it = 10;
    BOOST_TEST((end != it));
    BOOST_TEST((*it == 10));
    ++it;
    BOOST_TEST((it == end));
}

BOOST_AUTO_TEST_CASE(unique_container_const_iterator)
{
    acqua::container::unique_container_iterator<const std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_TEST((end == end));
    BOOST_TEST((it != end));
    BOOST_TEST((end != it));
    BOOST_TEST((*it == 1));
    BOOST_TEST((++it != end));
    BOOST_TEST((*it == 2));
    BOOST_TEST((it != end));
    ++it;
    BOOST_TEST((*it == 3));
    ++it;
    BOOST_TEST((end != ++it));
    BOOST_TEST((end != it));
    ++it;
    BOOST_TEST((it == end));
}

BOOST_AUTO_TEST_SUITE_END()
