#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

namespace my {
namespace type_traits {

  template <class T, T v>
  struct integral_constant
  {
    typedef T value_type;
    typedef integral_constant<T, v> type;
    enum { value = v };
    operator value_type() { return value; }
  };

  typedef integral_constant<bool, true> true_type;
  typedef integral_constant<bool, false> false_type;

  template <class T, class U>
  struct is_same { enum { value = 0 }; };
  template <class T>
  struct is_same<T, T> { enum { value = 1}; };

  template <class T>
  struct add_reference { typedef T& type; };
  template <class T>
  struct add_reference<T&> { typedef T& type; };

  template <class T>
  struct add_const { typedef const T type; };
  template <class T>
  struct add_const<const T> { typedef const T type; };

  template <class T>
  struct add_pointer { typedef T* type; };

}}

#endif

