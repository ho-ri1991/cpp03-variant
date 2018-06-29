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
//    BOOST_CHECK_EQUAL(*my::get_if<int>(var1), 42);
    BOOST_CHECK_EQUAL((void*)my::get_if<1>(&var1), (void*)NULL);
    BOOST_CHECK_EQUAL((void*)my::get_if<2>(&var1), (void*)NULL);
    var1 = CountTest1(1);
    BOOST_CHECK_EQUAL(var1.index(), 1);
    BOOST_CHECK_EQUAL(my::get<1>(var1).i, 1);
    BOOST_CHECK_EQUAL(my::get<CountTest1>(var1).i, 1);
    BOOST_CHECK_EQUAL(my::get_if<1>(&var1)->i, 1);
//    BOOST_CHECK_EQUAL(my::get_if<CountTest1>(var1)->i, 42);
//    BOOST_CHECK_EQUAL(my::get_if<CountTest2>(var1), NULL);
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
  }
  
  BOOST_CHECK_EQUAL(CountTest1::live_count, 0);
  BOOST_CHECK_EQUAL(CountTest2::live_count, 0);
}
BOOST_AUTO_TEST_SUITE_END()

