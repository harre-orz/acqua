#include <acqua/mref.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/utility.hpp>
#include <memory>
#include <functional>

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
    explicit moveable_object(int value_)
        : value(value_) {}

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
    BOOST_TEST(obj2.value == 100);
    BOOST_TEST(obj1.value == 0);
}

BOOST_AUTO_TEST_CASE(move_on_copy_wrapper_std_ptr_T)
{
    using acqua::utility::move_on_copy_wrapper;

    std::unique_ptr<int> obj1(new int(100));
    move_on_copy_wrapper<decltype(obj1)> wrap1(std::move(obj1));
    move_on_copy_wrapper<decltype(obj1)> wrap2 = wrap1;
    std::unique_ptr<int> obj2 = wrap2;
    BOOST_TEST(*obj2 == 100);
    BOOST_TEST(!obj1);
}

BOOST_AUTO_TEST_CASE(move_on_copy_wrapper_dereference_T)
{
    using acqua::utility::move_on_copy_wrapper;

    int * obj1(new int(100));
    move_on_copy_wrapper<decltype(obj1)> wrap1(std::move(obj1));
    move_on_copy_wrapper<decltype(obj1)> wrap2 = wrap1;
    int * obj2 = wrap2;
    BOOST_TEST(*obj2 == 100);
    //BOOST_TEST(!obj1);  // dereference のときは移動元を nullptr に初期化しないので、チェックしない
}


void func1(std::unique_ptr<int> obj) {
    BOOST_CHECK_EQUAL(*obj, 100);
}

BOOST_AUTO_TEST_CASE(mref)
{
    std::unique_ptr<int> obj(new int(100));
    std::bind(&func1, acqua::mref(std::move(obj)))();
    std::bind(&func1, acqua::mref(std::unique_ptr<int>(new int(100))))();
}

BOOST_AUTO_TEST_SUITE_END()
