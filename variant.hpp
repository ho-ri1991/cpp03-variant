#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <new>
#include <cassert>
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

    template <class TypeList, class Visitor, std::size_t I>
    struct variant_index_visit_impl
    {
      typedef variant_index_visit_impl<typename type_traits::tail<TypeList>::type, Visitor, I + 1> next;
      typedef typename type_traits::head<TypeList>::type T;
      static void visit_by_index(Visitor v, std::size_t d)
      {
        if (I == d)
          v.template visit<I>();
        else
          next::visit_by_index(v, d);
      }
    };
    template <class T, class Visitor, std::size_t I>
    struct variant_index_visit_impl<type_traits::type_list<T, type_traits::null_type>, Visitor, I>
    {
      static void visit_by_index(Visitor v, std::size_t d)
      {
        if (I == d)
          v.template visit<I>();
        else
          v.template visit<I + 1>();
      }
    };
    template <class TypeList, class Visitor>
    void variant_index_visit(Visitor v, std::size_t index) { variant_index_visit_impl<TypeList, Visitor, 0>::visit_by_index(v, index); }

    template <class TypeList, class Fn, std::size_t I>
    struct variant_overload_resolver_impl: variant_overload_resolver_impl<typename type_traits::tail<TypeList>::type, Fn, I + 1>
    {
      typedef variant_overload_resolver_impl<typename type_traits::tail<TypeList>::type, Fn, I + 1> base;
      typedef typename type_traits::head<TypeList>::type T;
      static void overload(const T& val, Fn fn) { fn.template invoke<I>(val); }
      using base::overload;
    };
    template <class T, class Fn, std::size_t I>
    struct variant_overload_resolver_impl<type_traits::type_list<T, type_traits::null_type>, Fn, I>
    {
      static void overload(const T& val, Fn fn) { fn.template invoke<I>(val); }
    };
    template <class TypeList, class Fn>
    struct variant_overload_resolver: variant_overload_resolver_impl<TypeList, Fn, 0> { using variant_overload_resolver_impl<TypeList, Fn, 0>::overload; };
    template <class TypeList, class T, class Fn>
    void variant_overload_resolve(const T& val, Fn fn) { variant_overload_resolver<TypeList, Fn>::overload(val, fn); }
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
      enum { type_num = type_traits::length<TypeList>::value };
    public:
      std::size_t get_discriminator() const { return discriminator; }
      void* get_raw_buffer() { return storage.address(); }
      const void* get_raw_buffer() const { return storage.address(); }
      template <class T>
      T* get_as() { return reinterpret_cast<T*>(storage.address()); }
      template <class T>
      const T* get_as() const { return reinterpret_cast<const T*>(storage.address()); }
    private:
      template <std::size_t I, class Dummy = void>
      struct destroy_visitor_impl
      {
        static void visit_impl(void* ptr) MY_NOEXCEPT
        {
          typedef typename type_traits::get<TypeList, I>::type T;
          static_cast<T*>(ptr)->~T();
        }
      };
      template <class Dummy>
      struct destroy_visitor_impl<type_num, Dummy> { static void visit_impl(void* ptr) MY_NOEXCEPT {} };
      struct destroy_visitor
      {
        void* ptr;
        destroy_visitor(void* ptr): ptr(ptr) {}
        template <std::size_t I>
        void visit() MY_NOEXCEPT { destroy_visitor_impl<I>::visit_impl(ptr); }
      };
      struct overload_initializer 
      {
        local_storage& storage;
        overload_initializer(local_storage& storage): storage(storage) {}
        template <std::size_t I, class T>
        void invoke(const T& val)
        {
          try
          {
            storage.ptr = new(storage.ptr) T(val);
            storage.discriminator = I;
          }
          catch(...)
          {
            storage.discriminator = variant_nops;
            throw;
          }
        }
      };
      struct overload_assigner
      {
        local_storage& storage;
        overload_assigner(local_storage& storage): storage(storage) {}
        template <std::size_t I, class T>
        void invoke(const T& val)
        {
          if (I == storage.discriminator)
          {
            *static_cast<T*>(storage.ptr) = val;
          }
          else
          {
            detail::variant_index_visit<TypeList>(destroy_visitor(storage.ptr), storage.discriminator);
            try
            {
              storage.ptr = new(storage.ptr) T(val);
              storage.discriminator = I;
            }
            catch(...)
            {
              storage.discriminator = variant_nops;
              throw;
            }
          }
        }
      };
      template <std::size_t I, class Dummy = void>
      struct copy_ctor_visitor_impl
      {
        typedef typename type_traits::get<TypeList, I>::type T;
        static void visit_impl(const local_storage& from, local_storage& to)
        {
          assert(I == from.discriminator);
          try
          {
            to.ptr = new(to.ptr) T(*from.get_as<T>());
            to.discriminator = I;
          }
          catch(...)
          {
            to.discriminator = variant_nops;
          }
        }
      };
      template <class Dummy>
      struct copy_ctor_visitor_impl<type_num, Dummy> { static void visit_impl(const local_storage& from, local_storage& to) { to.discriminator = variant_nops; } };
      struct copy_ctor_visitor
      {
        const local_storage& from;
        local_storage& to;
        copy_ctor_visitor(const local_storage& from, local_storage& to): from(from), to(to) {}
        template <std::size_t I>
        void visit() { copy_ctor_visitor_impl<I>::visit_impl(from , to); }
      };
      template <std::size_t I, class Dummy = void>
      struct copy_assign_visitor_impl
      {
        static void visit_impl(const local_storage& from, local_storage& to)
        {
          typedef typename type_traits::get<TypeList, I>::type T;
          assert(I == from.discriminator);
          if (from.discriminator == to.discriminator)
          {
            *to.get_as<T>() = *from.get_as<T>();
          }
          else
          {
            detail::variant_index_visit<TypeList>((destroy_visitor(to.ptr)), to.discriminator);
            try
            {
              to.ptr = new(to.ptr) T(*from.get_as<T>());
              to.discriminator = from.discriminator;
            }
            catch(...)
            {
              to.discriminator = variant_nops;
            }
          }
        }
      };
      template <class Dummy>
      struct copy_assign_visitor_impl<type_num, Dummy>
      { 
        static void visit_impl(const local_storage& from, local_storage& to)
        { 
          detail::variant_index_visit<TypeList>((destroy_visitor(to.ptr)), to.discriminator);
          to.discriminator = variant_nops;
        }
      };
      struct copy_assign_visitor
      {
        const local_storage& from;
        local_storage& to;
        copy_assign_visitor(const local_storage& from, local_storage& to): from(from), to(to) {}
        template <std::size_t I>
        void visit() { copy_assign_visitor_impl<I>::visit_impl(from, to); }
      };

    public:
      template <class T>
      local_storage(const T& val): storage(), ptr(storage.address())
      {
        detail::variant_overload_resolve<TypeList>(val, (overload_initializer(*this)));
      }
      local_storage(const local_storage& other): storage(), ptr(storage.address())
      {
        detail::variant_index_visit<TypeList>((copy_ctor_visitor(other, *this)), other.get_discriminator());
      }
      void destroy()
      {
        detail::variant_index_visit<TypeList>((destroy_visitor(ptr)), discriminator);
      }
      template <class T>
      void assign(const T& val)
      {
        detail::variant_overload_resolve<TypeList>(val, (overload_assigner(*this)));
      }
      local_storage& operator=(const local_storage& other)
      {
        detail::variant_index_visit<TypeList>((copy_assign_visitor(other, *this)), other.discriminator);
        return *this;
      }
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
      void destroy();
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

