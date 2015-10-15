#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <acqua/container/shared_container_iterator.hpp>
#include <acqua/container/unique_container_iterator.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(pointer_container_iterator)

BOOST_AUTO_TEST_CASE(shared_container_iterator)
{
    acqua::container::shared_container_iterator< std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_CHECK(end == end);
    BOOST_CHECK(it != end);
    BOOST_CHECK(end != it);
    BOOST_CHECK(*it == 1);
    BOOST_CHECK(++it != end);
    BOOST_CHECK(*it == 2);
    BOOST_CHECK(it++ != end);
    BOOST_CHECK(*it++ == 3);
    BOOST_CHECK(end != ++it);
    BOOST_CHECK(end != it++);
    BOOST_CHECK(it == end);
    BOOST_CHECK(it.get() != nullptr);
    BOOST_CHECK(end.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(shared_container_const_iterator)
{
    acqua::container::shared_container_iterator<const std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_CHECK(end == end);
    BOOST_CHECK(it != end);
    BOOST_CHECK(end != it);
    BOOST_CHECK(*it == 1);
    BOOST_CHECK(++it != end);
    BOOST_CHECK(*it == 2);
    BOOST_CHECK(it++ != end);
    BOOST_CHECK(*it++ == 3);
    BOOST_CHECK(end != ++it);
    BOOST_CHECK(end != it++);
    BOOST_CHECK(it == end);
    BOOST_CHECK(it.get() != nullptr);
    BOOST_CHECK(end.get() == nullptr);
}

BOOST_AUTO_TEST_CASE(unique_container_iterator_pointer)
{
    acqua::container::unique_container_iterator< std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_CHECK(end == end);
    BOOST_CHECK(it != end);
    BOOST_CHECK(end != it);
    BOOST_CHECK(*it == 1);
    BOOST_CHECK(++it != end);
    BOOST_CHECK(*it == 2);
    BOOST_CHECK(++it != end);
    BOOST_CHECK(*it == 3);
    ++it;
    BOOST_CHECK(end != ++it);
    *it = 10;
    BOOST_CHECK(end != it);
    BOOST_CHECK(10 == *it);
    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_CASE(unique_container_const_iterator)
{
    acqua::container::unique_container_iterator<const std::vector<int> > it(new std::vector<int>({1,2,3,4,5}));
    decltype(it) end;
    BOOST_CHECK(end == end);
    BOOST_CHECK(it != end);
    BOOST_CHECK(end != it);
    BOOST_CHECK(*it == 1);
    BOOST_CHECK(++it != end);
    BOOST_CHECK(*it == 2);
    BOOST_CHECK(it != end);
    ++it;
    BOOST_CHECK(*it == 3);
    ++it;
    BOOST_CHECK(end != ++it);
    BOOST_CHECK(end != it);
    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_SUITE_END()
