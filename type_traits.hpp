#ifndef TYPE_TRAITS_HPP
#define TYPE_TRAITS_HPP

namespace my {
namespace type_traits {

  struct true_type { enum { value = 1 }; };
  struct false_type { enum { value = 0 }; };

  template <class T, class U>
  struct is_same { enum { value = 0 }; };
  template <class T>
  struct is_same<T, T> { enum { value = 1}; };

}}

#endif

