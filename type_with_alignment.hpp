#ifndef TYPE_WITH_ALIGNMENT_HPP
#define TYPE_WITH_ALIGNMENT_HPP

#include "alignment_of.hpp"
#include "type_list.hpp"

namespace my {
namespace type_traits {

  namespace detail
  {
    struct alignment_dummy_type;
    typedef void (*function_ptr)();
    typedef int (alignment_dummy_type::*member_ptr);
    typedef int (alignment_dummy_type::*member_function_ptr)();

    union max_align
    {
      char c;
      short s;
      int i;
      long l;
      long long ll;
      float f;
      double d;
      long double ld;
      void* vp;
      function_ptr fp;
      member_ptr mp;
      member_function_ptr mfp;
    };

    template <std::size_t Target, class TypeList, bool check = alignment_of<typename head<TypeList>::type>::value == Target>
    struct type_with_alignment_impl{ typedef typename head<TypeList>::type type; };
    template <std::size_t Target, class TypeList>
    struct type_with_alignment_impl<Target, TypeList, false>: public type_with_alignment_impl<Target, typename tail<TypeList>::type>{};
    template <std::size_t Target, class T>
    struct type_with_alignment_impl<Target, type_list<T, null_type>, false> { typedef max_align type; };
  }

  template <std::size_t Alignment>
  struct type_with_alignment
  {
    typedef typename detail::type_with_alignment_impl<
      Alignment, 
      MAKE_MY_TYPE_LIST_12(char, short, int, long, long long, float, double, long double, void*, detail::function_ptr, detail::member_function_ptr, detail::member_ptr)
    >::type type;
  };

}}

#endif

