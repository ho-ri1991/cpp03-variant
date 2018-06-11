#ifndef VARIANT_HPP
#define VARIANT_HPP

#include "type_list.hpp"
#include "type_traits.hpp"
#include "alignment_of.hpp"
#include "alignment_storage.hpp"

namespace my {

  namespace detail {
    template <class TypeList>
    struct max_alignment_in_type_list
    {
    private:
      enum { head_alignment = alignment_of<typename head<TypeList>::type>::value };
      enum { tail_max_alignment = max_alignment_in_type_list<typename tail<TypeList>::type>::value };
    public:
      enum { value = head_alignment > tail_max_alignment ? head_alignment : tail_max_alignment };
    };
    template <class T>
    struct max_alignment_in_type_list<type_list<T, null_type>>
    {
      enum { value = alignment_of<T>::value; };
    };

    template <class TypeList>
    struct max_size_in_type_list
    {
    private:
      enum { head_size = sizeof(typename head<TypeList>::type) };
      enum { tail_max_size = max_alignment<typename tail<TypeList>::type>::value };
    public:
      enum { value = head_alignment > tail_max_alignment ? head_alignment : tail_max_alignment };
    };
    template <class T>
    struct max_size_in_type_list<type_list<T, null_type>
    {
      enum { value = sizeof(T) };
    };
  }

  template <class TypeList, class StoragePolicy>
  class variant {
  };
}

#endif

