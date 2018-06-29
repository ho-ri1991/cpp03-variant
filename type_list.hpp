#ifndef TYPE_LIST_HPP
#define TYPE_LIST_HPP

#include <cstddef>
#include "type_traits.hpp"

namespace my {
namespace type_traits {
  struct null_type {};

  template <class Head, class Tail>
  struct type_list
  {
    typedef Head head;
    typedef Tail tail;
  };

  template <class TypeList>
  struct head { typedef typename TypeList::head type; };
  
  template <class TypeList>
  struct tail { typedef typename TypeList::tail type; };

  template <class TypeList>
  struct length;
  template <class Head, class Tail>
  struct length<type_list<Head, Tail> >: integral_constant<std::size_t, length<Tail>::value + 1>{};
  template <class T>
  struct length<type_list<T, null_type> >: integral_constant<std::size_t, 1>{};

  template <class TypeList, std::size_t I>
  struct get: get<typename tail<TypeList>::type, I - 1> {};
  template <class TypeList>
  struct get<TypeList, 0> { typedef typename TypeList::head type; };

}}

#define MAKE_MY_TYPE_LIST_1(T1) ::my::type_traits::type_list<T1, ::my::type_traits::null_type>
#define MAKE_MY_TYPE_LIST_2(T1, T2) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_1(T2) >
#define MAKE_MY_TYPE_LIST_3(T1, T2, T3) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_2(T2, T3) >
#define MAKE_MY_TYPE_LIST_4(T1, T2, T3, T4) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_3(T2, T3, T4) >
#define MAKE_MY_TYPE_LIST_5(T1, T2, T3, T4, T5) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_4(T2, T3, T4, T5) >
#define MAKE_MY_TYPE_LIST_6(T1, T2, T3, T4, T5, T6) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_5(T2, T3, T4, T5, T6) >
#define MAKE_MY_TYPE_LIST_7(T1, T2, T3, T4, T5, T6, T7) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_6(T2, T3, T4, T5, T6, T7) >
#define MAKE_MY_TYPE_LIST_8(T1, T2, T3, T4, T5, T6, T7, T8) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_7(T2, T3, T4, T5, T6, T7, T8) >
#define MAKE_MY_TYPE_LIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >
#define MAKE_MY_TYPE_LIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >
#define MAKE_MY_TYPE_LIST_11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_10(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >
#define MAKE_MY_TYPE_LIST_12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) ::my::type_traits::type_list<T1, MAKE_MY_TYPE_LIST_11(T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) >

#endif

