#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <boost/utility.hpp>
#include <memory>

#include <acqua/utility/move_on_copy_wrapper.hpp>
struct moveable_object
{
    moveable_object()
        : value(0) {}
    moveable_object(moveable_object const &) = delete;
    moveable_object(moveable_object && rhs)
        : value(rhs.value)
    {
        rhs.value = 0;
    }
    explicit moveable_object(int value)
        : value(value) {}

    int value;
};

BOOST_AUTO_TEST_SUITE(move_on_copy_wrapper)

BOOST_AUTO_TEST_CASE(move_on_copy_wrapper_T)
{
    using acqua::utility::move_on_copy_wrapper;
    moveable_object obj1(100);
    move_on_copy_wrapper<moveable_object> wrap1(std::move(obj1));
    move_on_copy_wrapper<moveable_object> wrap2 = wrap1;
    moveable_object obj2 = wrap2;
    BOOST_CHECK_EQUAL(obj2.value, 100);
    BOOST_CHECK_EQUAL(obj1.value, 0);
}

BOOST_AUTO_TEST_CASE(move_on_copy_wrapper_std_ptr_T)
{
    using acqua::utility::move_on_copy_wrapper;

    std::unique_ptr<int> obj1(new int(100));
    move_on_copy_wrapper<decltype(obj1)> wrap1(std::move(obj1));
    move_on_copy_wrapper<decltype(obj1)> wrap2 = wrap1;
    std::unique_ptr<int> obj2 = wrap2;
    BOOST_CHECK_EQUAL(*obj2, 100);
    BOOST_CHECK_EQUAL((bool)obj1, false);
}

BOOST_AUTO_TEST_SUITE_END()
