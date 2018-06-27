#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <new>
#include "type_list.hpp"
#include "type_traits.hpp"
#include "alignment_of.hpp"
#include "aligned_storage.hpp"
#include "static_assert.hpp"

#define MY_NOEXCEPT throw()

namespace my {

  const std::size_t variant_nops = -1;

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

    template <class TypeList, std::size_t I, class T>
    T* variant_initializer_impl_helper(const T& val, void* ptr, std::size_t& d)
    {
      T* tmp = new(ptr) T(val);
      d = I;
      return tmp; 
    }
    template <class TypeList, std::size_t I>
    struct variant_initializer_impl: variant_initializer_impl<typename type_traits::tail<TypeList>::type, I + 1>
    {
      typedef variant_initializer_impl<typename type_traits::tail<TypeList>::type, I + 1> base;
      typedef typename type_traits::head<TypeList>::type T;
      static T* initialize(const T& val, void* ptr, std::size_t& d)
      {
        return variant_initializer_impl_helper<TypeList, I, T>(val, ptr, d);
      }
      using base::initialize;
    };
    template <class T, std::size_t I>
    struct variant_initializer_impl<type_traits::type_list<T, type_traits::null_type>, I>
    {
      typedef type_traits::type_list<T, type_traits::null_type> TypeList;
      static T* initialize(const T& val, void* ptr, std::size_t& d)
      {
        return variant_initializer_impl_helper<TypeList, I, T>(val, ptr, d);
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

    template <class TypeList, std::size_t I, class T>
    void* variant_assigner_impl_helper(const T& val, void* ptr, std::size_t& d)
    {
      void* tmp = ptr;
      if (d == I)
      {
        *static_cast<T*>(ptr) = val;
      }
      else
      {
        variant_destructor<TypeList>::destory(ptr, d);
        tmp = new(ptr) T(val);
        d = I;
      }
      return tmp;
    }
    template <class TypeList, std::size_t I>
    struct variant_assigner_impl: variant_assigner_impl<typename type_traits::tail<TypeList>::type, I + 1>
    {
      typedef variant_assigner_impl<typename type_traits::tail<TypeList>::type, I + 1> base;
      typedef typename type_traits::head<TypeList>::type T;
      static void* assign(const T& val, void* ptr, std::size_t& d)
      {
        return variant_initializer_impl_helper<TypeList, I, T>(val, ptr, d);
      }
      using base::assign;
    };
    template <class T, std::size_t I>
    struct variant_assigner_impl<type_traits::type_list<T, type_traits::null_type>, I>
    {
      typedef type_traits::type_list<T, type_traits::null_type> TypeList;
      static void* assign(const T& val, void* ptr, std::size_t& d)
      {
        return variant_assigner_impl_helper<TypeList, I, T>(val, ptr, d);
      }
    };
    template <class TypeList>
    struct variant_assigner: variant_assigner_impl<TypeList, 0>
    {
      using variant_assigner_impl<TypeList, 0>::assign;
    };

    template <class TypeList, class Variant, std::size_t I>
    struct variant_copy_initializer_impl
    {
      typedef variant_copy_initializer_impl<TypeList, Variant, I + 1> next;
      typedef typename type_traits::head<TypeList>::type T;
      static void* copy_initialize(const Variant& other, void* ptr, std::size_t& d)
      {
        if (I == other.index())
        {
          T* tmp = new(ptr) T(other.get());
          d = I;
          return tmp;
        }
        else
        {
          return next::copy_initialize(other, ptr, d);
        }
      }
    };
    template <class T, class Variant, std::size_t I>
    struct variant_copy_initializer_impl<type_traits::type_list<T, type_traits::null_type>, Variant, I>
    {
      static void* copy_initialize(const Variant& other, void* ptr, std::size_t& d)
      {
        if (I == other.index())
        {
          T* tmp = new(ptr) T(other.get());
          d = I;
          return tmp;
        }
        else
        {
          d = variant_nops;
          return NULL;
        }
      }
    };
    template <class TypeList, class Variant>
    struct variant_copy_initializer
    {
      static void* copy_initialize(const Variant& other, void* ptr, std::size_t& d)
      {
        return variant_copy_initializer_impl<TypeList, Variant, 0>::copy_initialize(other, ptr, d);
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
      void destory()
      {
        detail::variant_destructor<TypeList>::destory(ptr, discriminator);
      }
      template <class T>
      void assign(const T& val)
      {
        ptr = detail::variant_assigner<TypeList>::assign(val, storage.address(), discriminator);
      }
      std::size_t get_discriminator() const { return discriminator; }
      void* get_raw_buffer() { return storage.address(); }
      const void* get_raw_buffer() const { return storage.address(); }
      template <class T>
      T* get_as() { return reinterpret_cast<T*>(storage.address()); }
      template <class T>
      const T* get_as() const { return reinterpret_cast<const T*>(storage.address()); }
    };
    template <class TypeList>
    class dynamic_storage
    {
      void* ptr;
      std::size_t discriminator;
    public:
      template <class T>
      dynamic_storage(const T& val);
      template <class T>
      void assign(const T& val);
      void destory();
      std::size_t get_discriminator() const MY_NOEXCEPT { return discriminator; }
      void* get_raw_buffer() MY_NOEXCEPT { return ptr; }
      const void* get_raw_buffer() const MY_NOEXCEPT { return ptr; }
      template <class T>
      T* get_as() MY_NOEXCEPT { return reinterpret_cast<T*>(ptr); }
      template <class T>
      const T* get_as() const MY_NOEXCEPT { return reinterpret_cast<const T*>(ptr); }
    };
  }

  template <class TypeList, template <class> class Storage>
  class variant {
  private:
    typedef TypeList type_list;
    Storage<TypeList> storage;
    typedef typename type_traits::head<TypeList>::type head_type;
    template <std::size_t I>
    typename type_traits::add_reference<typename type_traits::get<TypeList, I>::type>::type get();
    template <std::size_t I>
    typename type_traits::add_reference<typename type_traits::add_const<typename type_traits::get<TypeList, I>::type>::type>::type get() const;
  public:
    std::size_t index() const MY_NOEXCEPT { return storage.get_discriminator(); }
    variant(): storage((head_type())) {}
    variant(const variant& other) {  }
  };
}

#endif

