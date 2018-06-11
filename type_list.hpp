#ifndef TYPE_LIST_HPP
#define TYPE_LIST_HPP

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
  struct length<TypeList<Head, Tail>>
  {
    enum
    {
      value = Length<Tail>::value + 1;
    };
  };
  template <class T>
  struct length<TypeList<T, null_type>>
  {
    enum
    {
      value = 1;
    };
  };

  template <class TypeList, std::size_t I>
  struct get: get<TypeList::tail, I - 1> {};
  template <class TypeList>
  struct get<TypeList, 0> { typedef typename TypeList::head type; }

}}

#endif

