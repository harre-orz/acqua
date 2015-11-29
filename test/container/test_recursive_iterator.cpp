#include <acqua/container/recursive_iterator.hpp>
#include <boost/test/included/unit_test.hpp>
#include <list>

struct node {
    std::list<node> children;
    using iterator = std::list<node>::iterator ;
    iterator begin()
    {
        return children.begin();
    }
    iterator end()
    {
        return children.end();
    }
};

node::iterator begin(node & node)
{
    return node.begin();
}

node::iterator end(node & node)
{
    return node.end();
}


BOOST_AUTO_TEST_SUITE(recursive_iterator)

BOOST_AUTO_TEST_CASE(preordered_recursive_iterator)
{
    struct node n;
    n.children.push_back(node());
    n.children.push_back(node());
    n.children.push_back(node());
    n.children.front().children.push_back(node());
    acqua::container::preordered_recursive_iterator<decltype(n), typename decltype(n.children)::iterator> it(n);
    ++it;
    BOOST_TEST(it.depth() == 1);
    ++it;
    BOOST_TEST(it.depth() == 2);
    ++it;
    BOOST_TEST(it.depth() == 3);
    ++it;
    BOOST_TEST(it.depth() == 4);
    ++it;
    BOOST_TEST(it.depth() == 5);
}


BOOST_AUTO_TEST_SUITE_END()
