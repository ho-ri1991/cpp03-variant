#ifndef TYPE_WITH_ALIGNMENT_HPP
#define TYPE_WITH_ALIGNMENT_HPP

#include "alignment_of.hpp"

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

    template <std::size_t Target, bool check = alignment_of<long double>::value == Target>
    struct long_double_alignment { typedef long double type; };
    template <std::size_t Target>
    struct long_double_alignment<Target, false> { typedef max_align type; };

    template <std::size_t Target, bool check = alignment_of<double>::value == Target>
    struct double_alignment { typedef double type; };
    template <std::size_t Target>
    struct double_alignment<Target, false> { typedef typename long_double_alignment<Target>::type type; };

    template <std::size_t Target, bool check = alignment_of<long long>::value == Target>
    struct long_long_alignment { typedef long long type; };
    template <std::size_t Target>
    struct long_long_alignment<Target, false> { typedef typename double_alignment<Target>::type type; };

    template <std::size_t Target, bool check = alignment_of<long>::value == Target>
    struct long_alignment { typedef long type; };
    template <std::size_t Target>
    struct long_alignment<Target, false> { typedef typename long_long_alignment<Target>::type type; };

    template <std::size_t Target, bool check = alignment_of<int>::value == Target>
    struct int_alignment { typedef int type; };
    template <std::size_t Target>
    struct int_alignment<Target, false> { typedef typename long_alignment<Target>::type type; };

    template <std::size_t Target, bool check = alignment_of<short>::value == Target>
    struct short_alignment { typedef short type; };
    template <std::size_t Target>
    struct short_alignment<Target, false> { typedef typename int_alignment<Target>::type type; };

    template <std::size_t Target, bool check = alignment_of<char>::value == Target>
    struct char_alignment { typedef char type; };
    template <std::size_t Target>
    struct char_alignment<Target, false> { typedef typename short_alignment<Target>::type type; };
  }

  template <std::size_t Alignment>
  struct type_with_alignment
  {
    typedef typename detail::char_alignment<Alignment>::type type;
  };

}}

#endif

