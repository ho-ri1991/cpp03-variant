#ifndef ALIGNMENT_OF_HPP
#define ALIGNMENT_OF_HPP

#include <cstddef>

namespace my {
namespace type_traits {

  namespace detail
  {
    template <class T>
    struct alignment_hack
    {
      char c;
      T t;
    };

    template <std::size_t N, std::size_t M>
    struct min_value
    {
      enum { value = N < M ? N : M };
    };
  }

  template <class T>
  struct alignment_of: detail::min_value<sizeof(T), sizeof(detail::alignment_hack<T>) - sizeof(T)>{};

}}

#endif

