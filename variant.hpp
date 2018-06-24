#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <new>
#include "type_list.hpp"
#include "type_traits.hpp"
#include "alignment_of.hpp"
#include "aligned_storage.hpp"
#include "static_assert.hpp"

namespace my {

  namespace detail
  {
    template <class TypeList>
    struct max_alignment_in_type_list
    {
    private:
      enum { head_alignment = type_traits::alignment_of<typename type_traits::head<TypeList>::type>::value };
      enum { tail_max_alignment = max_alignment_in_type_list<typename type_traits::tail<TypeList>::type>::value };
    public:
      enum { value = head_alignment > tail_max_alignment ? head_alignment : tail_max_alignment };
    };
    template <class T>
    struct max_alignment_in_type_list<type_traits::type_list<T, type_traits::null_type> >
    {
      enum { value = type_traits::alignment_of<T>::value };
    };

    template <class TypeList>
    struct max_size_in_type_list
    {
    private:
      enum { head_size = sizeof(typename type_traits::head<TypeList>::type) };
      enum { tail_max_size = max_size_in_type_list<typename type_traits::tail<TypeList>::type>::value };
    public:
      enum { value = head_size > tail_max_size ? head_size : tail_max_size };
    };
    template <class T>
    struct max_size_in_type_list<type_traits::type_list<T, type_traits::null_type> >
    {
      enum { value = sizeof(T) };
    };

    template <std::size_t Alignment, class TypeList, bool = Alignment % type_traits::alignment_of<typename type_traits::head<TypeList>::type>::value>
    struct is_aligned_for_all: type_traits::false_type{};
    template <std::size_t Alignment, class TypeList>
    struct is_aligned_for_all<Alignment, TypeList, false>: is_aligned_for_all<Alignment, typename type_traits::tail<TypeList>::type>{};
    template <std::size_t Alignment, class T>
    struct is_aligned_for_all<Alignment, type_traits::type_list<T, type_traits::null_type>, false>: type_traits::true_type{};

    template <class TypeList, std::size_t I>
    struct variant_initializer_impl: variant_initializer_impl<typename type_traits::tail<TypeList>::type, I + 1>
    {
      typedef variant_initializer_impl<typename type_traits::tail<TypeList>::type, I + 1> base;
      typedef typename type_traits::head<TypeList>::type T;
      static T* initialize(const T& val, void* ptr, std::size_t& d)
      {
        T* tmp = new(ptr) T(val);
        d = I;
        return tmp; 
      }
      using base::initialize;
    };
    template <class T, std::size_t I>
    struct variant_initializer_impl<type_traits::type_list<T, type_traits::null_type>, I>
    {
      static T* initialize(const T& val, void* ptr, std::size_t& d)
      {
        T* tmp = new(ptr) T(val);
        d = I;
        return tmp;
      }
    };
    template <class TypeList>
    struct variant_initializer: variant_initializer_impl<TypeList, 0>
    {
      using variant_initializer_impl<TypeList, 0>::initialize;
    };

    template <class TypeList, std::size_t I>
    struct variant_destructor_impl
    {
      typedef variant_destructor_impl<typename type_traits::tail<TypeList>::type, I + 1> next;
      typedef typename type_traits::head<TypeList>::type T;
      static void destory(void* ptr, std::size_t d)
      {
        if (d == I)
          static_cast<T*>(ptr)->~T();
        else
          next::destory(ptr, d);
      }
    };
    template <class T, std::size_t I>
    struct variant_destructor_impl<type_traits::type_list<T, type_traits::null_type>, I>
    {
      static void destory(void* ptr, std::size_t d)
      {
        if (I == d) static_cast<T*>(ptr)->~T();
      }
    };
    template <class TypeList>
    struct variant_destructor
    {
      static void destory(void* ptr, std::size_t d)
      {
        variant_destructor_impl<TypeList, 0>::destory(ptr, d);
      }
    };
  }

  namespace variant_storage
  {
    template <class TypeList>
    class local_storage
    {
      enum { max_size = detail::max_size_in_type_list<TypeList>::value };
      enum { max_alignment = detail::max_alignment_in_type_list<TypeList>::value };
      type_traits::aligned_storage<max_size, max_alignment> storage;
      void* ptr;
      std::size_t discriminator;
      enum { is_aligned_for_all = detail::is_aligned_for_all<max_alignment, TypeList>::value };
      STATIC_ASSERT(is_aligned_for_all == 1, variant_storage_alignment_failure);
    public:
      template <class T>
      local_storage(const T& val)
      {
        ptr = detail::variant_initializer<TypeList>::initialize(val, storage.address(), discriminator);
      }
      void destory() { detail::variant_destructor<TypeList>::destory(ptr, discriminator); }
      std::size_t get_discriminator() const { return discriminator; }
      void* get_raw_buffer() { return storage.address(); }
      const void* get_raw_buffer() const { return storage.address(); }
      template <class T>
      T* get_as() { return reinterpret_cast<T*>(storage.address()); }
      template <class T>
      const T* get_as() const { return reinterpret_cast<const T*>(storage.address()); }
    };
  }

  template <class TypeList, class StoragePolicy>
  class variant {
  };
}

#endif

