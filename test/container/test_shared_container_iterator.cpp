#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <acqua/container/shared_container_iterator.hpp>
#include <vector>

BOOST_AUTO_TEST_SUITE(shared_container_iterator)

BOOST_AUTO_TEST_CASE(shared_container_iterator_mutable)
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
}

BOOST_AUTO_TEST_CASE(shared_container_iterator_constant)
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
}

BOOST_AUTO_TEST_SUITE_END()
