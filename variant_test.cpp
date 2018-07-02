#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <cassert>
#include <string>
#include "variant.hpp"
#include "static_assert.hpp"

STATIC_ASSERT((my::variant_size<my::variant< MAKE_MY_TYPE_LIST_1(int) > >::value == 1), variant_1_size_test);
STATIC_ASSERT((my::variant_size<my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::value == 3), variant_3_size_test);
STATIC_ASSERT((my::variant_size<const my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::value == 3), const_variant_3_size_test);
STATIC_ASSERT((my::variant_size<volatile my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::value == 3), volatile_variant_3_size_test);
STATIC_ASSERT((my::variant_size<const volatile my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::value == 3), cv_variant_3_size_test);

STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<0, my::variant< MAKE_MY_TYPE_LIST_1(int) > >::type,
    int
  >::value == 1),
  variant_1_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<0, my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    int
  >::value == 1),
  variant_3_0_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<1, my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    bool
  >::value == 1),
  variant_3_1_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<2, my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    double
  >::value == 1),
  variant_3_2_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<0, const my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    int
  >::value == 1),
  const_variant_3_0_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<1, volatile my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    bool
  >::value == 1),
  volatile_variant_3_1_alternative_test
);
STATIC_ASSERT((
  my::type_traits::is_same<
    my::variant_alternative<2, const volatile my::variant< MAKE_MY_TYPE_LIST_3(int, bool, double) > >::type,
    double
  >::value == 1),
  cv_variant_3_2_alternative_test
);

template <class T>
struct int_count_pred: my::type_traits::false_type{};
template <>
struct int_count_pred<int>: my::type_traits::true_type{};
STATIC_ASSERT((
  my::type_traits::count_if<MAKE_MY_TYPE_LIST_6(int, bool, int, const int, double, int), int_count_pred>::value == 3),
  count_if_test
);

STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), int>::value == 0),
    find_1st_test
);
STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), bool>::value == 1),
    find_2nd_test
);
STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), double>::value == 3),
    find_3rd_test
);
STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), float>::value == 4),
    find_4th_test
);
STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), void*>::value == 5),
    find_5th_test
);
STATIC_ASSERT(
    (my::type_traits::find<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), long>::value == 6),
    find_non_test
);

template <class T>
struct const_find_if_pred: my::type_traits::false_type {};
template <class T>
struct const_find_if_pred<const T>: my::type_traits::true_type {};
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(const int, bool, int, double, float, void*), const_find_if_pred>::value == 0),
    find_if_1_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, const bool, int, double, float, void*), const_find_if_pred>::value == 1),
    find_if_2_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, bool, const int, double, float, void*), const_find_if_pred>::value == 2),
    find_if_3_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, bool, int, const double, float, void*), const_find_if_pred>::value == 3),
    find_if_4_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, bool, int, double, const float, void*), const_find_if_pred>::value == 4),
    find_if_5_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void* const), const_find_if_pred>::value == 5),
    find_if_6_test
);
STATIC_ASSERT(
    (my::type_traits::find_if<MAKE_MY_TYPE_LIST_6(int, bool, int, double, float, void*), const_find_if_pred>::value == 6),
    find_if_non_test
);

struct CountTest1
{
  int i;
  static int live_count;
  CountTest1(int i):i(i) { ++live_count; }
  CountTest1(const CountTest1& other): i(other.i) { ++live_count; }
  ~CountTest1() { --live_count; }
};
int CountTest1::live_count = 0;

struct CountTest2
{
  bool i;
  static int live_count;
  CountTest2(bool i):i(i) { ++live_count; }
  CountTest2(const CountTest2& other): i(other.i) { ++live_count; }
  ~CountTest2() { --live_count; }
};
int CountTest2::live_count = 0;

BOOST_AUTO_TEST_SUITE(my_variant)
BOOST_AUTO_TEST_CASE(my_variant_local_storage) {

  BOOST_CHECK_EQUAL(CountTest1::live_count, 0);
  BOOST_CHECK_EQUAL(CountTest2::live_count, 0);

  {
    my::variant< MAKE_MY_TYPE_LIST_3(int, CountTest1, CountTest2) > var1(42);
    BOOST_CHECK_EQUAL(var1.index(), 0);
    BOOST_CHECK_EQUAL(my::get<0>(var1), 42);
    BOOST_CHECK_EQUAL(my::get<int>(var1), 42);
    BOOST_CHECK_EQUAL(*my::get_if<0>(&var1), 42);
    BOOST_CHECK_EQUAL(*my::get_if<int>(&var1), 42);
    BOOST_CHECK_EQUAL((void*)my::get_if<1>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<2>(&var1), (void*)NULL);

    var1 = CountTest1(1);
    BOOST_CHECK_EQUAL(var1.index(), 1);
    BOOST_CHECK_EQUAL(my::get<1>(var1).i, 1);
    BOOST_CHECK_EQUAL(my::get<CountTest1>(var1).i, 1);
    BOOST_CHECK_EQUAL(my::get_if<1>(&var1)->i, 1);
    BOOST_CHECK_EQUAL(my::get_if<CountTest1>(&var1)->i, 1);
    BOOST_CHECK_EQUAL((void*)my::get_if<CountTest2>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<0>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<2>(&var1), (void*)NULL);

    var1 = CountTest2(true);
    BOOST_CHECK_EQUAL(var1.index(), 2);
    BOOST_CHECK_EQUAL(my::get<2>(var1).i, true);
    BOOST_CHECK_EQUAL(my::get<CountTest2>(var1).i, true);

    var1 = 1;
    BOOST_CHECK_EQUAL(var1.index(), 0);
    BOOST_CHECK_EQUAL(my::get<0>(var1), 1);
    BOOST_CHECK_EQUAL(my::get<int>(var1), 1);
    
    var1 = CountTest2(true);
    BOOST_CHECK_EQUAL(var1.index(), 2);
    BOOST_CHECK_EQUAL(my::get<2>(var1).i, true);
    BOOST_CHECK_EQUAL(my::get<CountTest2>(var1).i, true);

    var1.emplace((my::in_place_index_t<1>()), 42);
    BOOST_CHECK_EQUAL(var1.index(), 1);
    BOOST_CHECK_EQUAL(my::get<1>(var1).i, 42);
    BOOST_CHECK_EQUAL(my::get<CountTest1>(var1).i, 42);
    BOOST_CHECK_EQUAL(my::get_if<1>(&var1)->i, 42);
    BOOST_CHECK_EQUAL(my::get_if<CountTest1>(&var1)->i, 42);
    BOOST_CHECK_EQUAL((void*)my::get_if<CountTest2>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<0>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<2>(&var1), (void*)NULL);
    
    my::variant< MAKE_MY_TYPE_LIST_3(int, CountTest1, CountTest2) > var2((my::in_place_type_t<CountTest1>()), 42);
    BOOST_CHECK_EQUAL(var2.index(), 1);
    BOOST_CHECK_EQUAL(my::get<1>(var2).i, 42);
    BOOST_CHECK_EQUAL(my::get<CountTest1>(var2).i, 42);
    BOOST_CHECK_EQUAL(my::get_if<1>(&var2)->i, 42);
    BOOST_CHECK_EQUAL(my::get_if<CountTest1>(&var2)->i, 42);
    BOOST_CHECK_EQUAL((void*)my::get_if<CountTest2>(&var2), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<0>(&var2), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<2>(&var2), (void*)NULL);
  }

  BOOST_CHECK_EQUAL(CountTest2::live_count, 0);
  BOOST_CHECK_EQUAL(CountTest1::live_count, 0);
}
BOOST_AUTO_TEST_SUITE_END()

