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
  template <class T>
  struct tail<type_list<T, null_type> > {};

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

  namespace detail
  {
    template <class TypeList, class T>
    struct count_impl
    {
    private:
      typedef typename head<TypeList>::type Head;
      typedef typename tail<TypeList>::type Tail;
      enum { tail_value = count_impl<Tail, T>::value };
      enum { head_value = is_same<Head, T>::value };
    public:
      enum { value = head_value + tail_value };
    };
    template <class T, class U>
    struct count_impl<type_list<T, null_type>, U>
    {
    public:
      enum { value = is_same<T, U>::value };
    };
    
    template <class TypeList, template <class> class Predicate>
    struct count_if_impl
    {
    private:
      typedef typename head<TypeList>::type Head;
      typedef typename tail<TypeList>::type Tail;
      enum { tail_value = count_if_impl<Tail, Predicate>::value };
      enum { head_value = Predicate<Head>::value };
    public:
      enum { value = head_value + tail_value };
    };
    template <class T, template <class> class Predicate>
    struct count_if_impl<type_list<T, null_type>, Predicate>
    {
    public:
      enum { value = Predicate<T>::value };
    };

    template <class TypeList, class T, std::size_t I>
    struct find_impl
    {
    private:
      typedef typename head<TypeList>::type Head;
      typedef typename tail<TypeList>::type Tail;
      typedef find_impl<Tail, T, I + 1> Next;
      typedef conditional<is_same<Head, T>::value == 1, integral_constant<std::size_t, I>, Next> Result;
    public:
      enum { value = Result::type::value };
    };
    template <class T, class U, std::size_t I>
    struct find_impl<type_list<T, null_type>, U, I>
    {
      enum { value = is_same<T, U>::value == 1 ? I : I + 1 };
    };

    template <class TypeList, template <class> class Predicate, std::size_t I>
    struct find_if_impl
    {
    private:
      typedef typename head<TypeList>::type Head;
      typedef typename tail<TypeList>::type Tail;
      typedef find_if_impl<Tail, Predicate, I + 1> Next;
      typedef conditional<Predicate<Head>::value == 1, integral_constant<std::size_t, I>, Next> Result;
    public:
      enum { value = Result::type::value };
    };
    template <class T, template <class> class Predicate, std::size_t I>
    struct find_if_impl<type_list<T, null_type>, Predicate, I>
    {
      enum { value = Predicate<T>::value == 1 ? I : I + 1 };
    };
  }

  template <class TypeList, class T>
  struct count: integral_constant<std::size_t, detail::count_impl<TypeList, T>::value> {};

  template <class TypeList, template <class> class Predicate>
  struct count_if: integral_constant<std::size_t, detail::count_if_impl<TypeList, Predicate>::value> {};

  template <class TypeList, class T>
  struct find: integral_constant<std::size_t, detail::find_impl<TypeList, T, 0>::value> {};

  template <class TypeList, template <class> class Predicate>
  struct find_if: integral_constant<std::size_t, detail::find_if_impl<TypeList, Predicate, 0>::value> {};

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

