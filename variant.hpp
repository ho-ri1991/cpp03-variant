#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <new>
#include <cassert>
#include <exception>
#include "type_list.hpp"
#include "type_traits.hpp"
#include "alignment_of.hpp"
#include "aligned_storage.hpp"
#include "static_assert.hpp"

#define MY_NOEXCEPT throw()

namespace my {

  // forward declaration
  template <class TypeList, template <class> class Storage>
  class variant;

  namespace {
    const std::size_t variant_nops = -1;
  }

  template <class T>
  struct variant_size;
  template <class T>
  struct variant_size<const T>: variant_size<T>{};
  template <class T>
  struct variant_size<volatile T>: variant_size<T>{};
  template <class T>
  struct variant_size<const volatile T>: variant_size<T>{};
  template <class TypeList, template <class> class Storage>
  struct variant_size< variant<TypeList, Storage> >: type_traits::length<TypeList>{};

  template <std::size_t I, class T>
  struct variant_alternative;
  template <std::size_t I, class T>
  struct variant_alternative<I, const T>: variant_alternative<I, T>{};
  template <std::size_t I, class T>
  struct variant_alternative<I, volatile T>: variant_alternative<I, T>{};
  template <std::size_t I, class T>
  struct variant_alternative<I, const volatile T>: variant_alternative<I, T>{};
  template <std::size_t I, class TypeList, template <class> class Storage>
  struct variant_alternative<I, variant<TypeList, Storage> > { typedef typename type_traits::get<TypeList, I>::type type; };

  template <class T>
  struct in_place_type_t {};
  template <std::size_t I>
  struct in_place_index_t {};

  class bad_variant_access: public std::exception
  {
    const char* what() const MY_NOEXCEPT { return "bad_variant_access"; }
  };

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
        {
          v.template visit<I>();
          return;
        }
        else
        {
          next::visit_by_index(v, d);
          return;
        }
      }
    };
    template <class T, class Visitor, std::size_t I>
    struct variant_index_visit_impl<type_traits::type_list<T, type_traits::null_type>, Visitor, I>
    {
      static void visit_by_index(Visitor v, std::size_t d)
      {
        if (I == d)
        {
          v.template visit<I>();
          return;
        }
        else
        {
          v.template visit<I + 1>();
          return;
        }
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

    template <class T, class TypeList, std::size_t I, bool is_last = I == type_traits::length<TypeList>::value>
    struct is_match_type_index_visitor_impl
    {
      typedef typename type_traits::get<TypeList, I>::type U;
      static bool visit_impl() MY_NOEXCEPT { return type_traits::is_same<T, U>::value; }
    };
    template <class T, class TypeList, std::size_t I>
    struct is_match_type_index_visitor_impl<T, TypeList, I, true>
    {
      static bool visit_impl() MY_NOEXCEPT { return false; }
    };
    template <class T, class TypeList>
    struct is_match_type_index_visitor
    {
      bool& result;
      is_match_type_index_visitor(bool& result): result(result) {}
      template <std::size_t I>
      void visit() MY_NOEXCEPT { result = is_match_type_index_visitor_impl<T, TypeList, I>::visit_impl(); }
    };
    template <class T, class TypeList>
    bool is_match_type_index(std::size_t d)
    {
      bool result = false;
      variant_index_visit<TypeList>((is_match_type_index_visitor<T, TypeList>(result)), d);
      return result;
    }
  }

  namespace variant_storage
  {
    template <class TypeList>
    class local_storage
    {
      enum { max_size = detail::max_size_in_type_list<TypeList>::value };
      enum { max_alignment = detail::max_alignment_in_type_list<TypeList>::value };
      type_traits::aligned_storage<max_size, max_alignment> storage;
      void* ptr; // this is for avoiding UB, std::launder in C++17 is required to remove this.
      std::size_t discriminator;
      enum { is_aligned_for_all = detail::is_aligned_for_all<max_alignment, TypeList>::value };
      STATIC_ASSERT(is_aligned_for_all == 1, local_storage_alignment_failuer);
      enum { type_num = type_traits::length<TypeList>::value };
    public:
      std::size_t get_discriminator() const { return discriminator; }
      void* get_raw_buffer() { return ptr; }
      const void* get_raw_buffer() const { return ptr; }
      template <class T>
      T* get_as() { return reinterpret_cast<T*>(ptr); }
      template <class T>
      const T* get_as() const { return reinterpret_cast<const T*>(ptr); }
    private:
      template <std::size_t I, class Dummy = void>
      struct destroy_visitor_impl
      {
        static void visit_impl(local_storage& storage) MY_NOEXCEPT
        {
          typedef typename type_traits::get<TypeList, I>::type T;
          storage.get_as<T>()->~T();
          storage.discriminator = variant_nops;
        }
      };
      template <class Dummy>
      struct destroy_visitor_impl<type_num, Dummy> { static void visit_impl(local_storage& storage) MY_NOEXCEPT { storage.discriminator = variant_nops; } };
      struct destroy_visitor
      {
        local_storage& storage;
        destroy_visitor(local_storage& storage): storage(storage) {}
        template <std::size_t I>
        void visit() MY_NOEXCEPT { destroy_visitor_impl<I>::visit_impl(storage); }
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
            detail::variant_index_visit<TypeList>(destroy_visitor(storage), storage.discriminator);
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
            detail::variant_index_visit<TypeList>((destroy_visitor(to)), to.discriminator);
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
          detail::variant_index_visit<TypeList>((destroy_visitor(to)), to.discriminator);
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
      template <std::size_t I, class T>
      local_storage(in_place_index_t<I>, const T& val): storage(), ptr(storage.address())
      {
        (overload_initializer(*this)).template invoke<I>(val);
      }
      local_storage(const local_storage& other): storage(), ptr(storage.address())
      {
        detail::variant_index_visit<TypeList>((copy_ctor_visitor(other, *this)), other.get_discriminator());
      }
      void destroy()
      {
        detail::variant_index_visit<TypeList>((destroy_visitor(*this)), discriminator);
      }
      template <class T>
      local_storage& operator=(const T& val)
      {
        detail::variant_overload_resolve<TypeList>(val, (overload_assigner(*this)));
        return *this;
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
      enum { type_num = type_traits::length<TypeList>::value };
    public:
      std::size_t get_discriminator() const MY_NOEXCEPT { return discriminator; }
      void* get_raw_buffer() MY_NOEXCEPT { return ptr; }
      const void* get_raw_buffer() const MY_NOEXCEPT { return ptr; }
      template <class T>
      T* get_as() MY_NOEXCEPT { return reinterpret_cast<T*>(ptr); }
      template <class T>
      const T* get_as() const MY_NOEXCEPT { return reinterpret_cast<const T*>(ptr); }
    private:
      template <std::size_t I, class Dummy = void>
      struct destroy_visitor_impl
      {
        static void visit_impl(dynamic_storage& storage) MY_NOEXCEPT
        {
          typedef typename type_traits::get<TypeList, I>::type T;
          delete static_cast<T*>(storage.ptr);
          storage.ptr = NULL;
          storage.discriminator = variant_nops;
        }
      };
      template <class Dummy>
      struct destroy_visitor_impl<type_num, Dummy>
      {
        static void visit_impl(dynamic_storage& storage) MY_NOEXCEPT
        {
          storage.ptr = NULL;
          storage.discriminator = variant_nops;
        }
      };
      struct destroy_visitor
      {
        dynamic_storage& storage;
        destroy_visitor(dynamic_storage& storage): storage(storage) {}
        template <std::size_t I>
        void visit() MY_NOEXCEPT { destroy_visitor_impl<I>::visit_impl(storage); }
      };
      struct overload_initializer 
      {
        dynamic_storage& storage;
        overload_initializer(dynamic_storage& storage): storage(storage) {}
        template <std::size_t I, class T>
        void invoke(const T& val)
        {
          try
          {
            storage.ptr = new T(val);
            storage.discriminator = I;
          }
          catch(...)
          {
            storage.ptr = NULL;
            storage.discriminator = variant_nops;
            throw;
          }
        }
      };
      struct overload_assigner
      {
        dynamic_storage& storage;
        overload_assigner(dynamic_storage& storage): storage(storage) {}
        template <std::size_t I, class T>
        void invoke(const T& val)
        {
          if (I == storage.discriminator)
          {
            *static_cast<T*>(storage.ptr) = val;
          }
          else
          {
            detail::variant_index_visit<TypeList>(destroy_visitor(storage), storage.discriminator);
            storage.ptr = new(storage.ptr) T(val);
            storage.discriminator = I;
          }
        }
      };
      template <std::size_t I, class Dummy = void>
      struct copy_ctor_visitor_impl
      {
        typedef typename type_traits::get<TypeList, I>::type T;
        static void visit_impl(const dynamic_storage& from, dynamic_storage& to)
        {
          assert(I == from.discriminator);
          try
          {
            to.ptr = new T(*from.get_as<T>());
            to.discriminator = I;
          }
          catch(...)
          {
            to.ptr = NULL;
            to.discriminator = variant_nops;
          }
        }
      };
      template <class Dummy>
      struct copy_ctor_visitor_impl<type_num, Dummy> { static void visit_impl(const dynamic_storage& from, dynamic_storage& to) { to.discriminator = variant_nops; } };
      struct copy_ctor_visitor
      {
        const dynamic_storage& from;
        dynamic_storage& to;
        copy_ctor_visitor(const dynamic_storage& from, dynamic_storage& to): from(from), to(to) {}
        template <std::size_t I>
        void visit() { copy_ctor_visitor_impl<I>::visit_impl(from , to); }
      };
      template <std::size_t I, class Dummy = void>
      struct copy_assign_visitor_impl
      {
        static void visit_impl(const dynamic_storage& from, dynamic_storage& to)
        {
          typedef typename type_traits::get<TypeList, I>::type T;
          assert(I == from.discriminator);
          if (from.discriminator == to.discriminator)
          {
            *to.get_as<T>() = *from.get_as<T>();
          }
          else
          {
            detail::variant_index_visit<TypeList>((destroy_visitor(to)), to.discriminator);
            to.ptr = new T(*from.get_as<T>());
            to.discriminator = from.discriminator;
          }
        }
      };
      template <class Dummy>
      struct copy_assign_visitor_impl<type_num, Dummy>
      { 
        static void visit_impl(const dynamic_storage& from, dynamic_storage& to)
        { 
          detail::variant_index_visit<TypeList>((destroy_visitor(to)), to.discriminator);
        }
      };
      struct copy_assign_visitor
      {
        const dynamic_storage& from;
        dynamic_storage& to;
        copy_assign_visitor(const dynamic_storage& from, dynamic_storage& to): from(from), to(to) {}
        template <std::size_t I>
        void visit() { copy_assign_visitor_impl<I>::visit_impl(from, to); }
      };

    public:
      template <class T>
      dynamic_storage(const T& val)
      {
        detail::variant_overload_resolve<TypeList>(val, (overload_initializer(*this)));
      }
      template <std::size_t I, class T>
      dynamic_storage(in_place_index_t<I>, const T& val)
      {
        (overload_initializer(*this)).template invoke<I>(val);
      }
      dynamic_storage(const dynamic_storage& other)
      {
        detail::variant_index_visit<TypeList>((copy_ctor_visitor(other, *this)), other.get_discriminator());
      }
      void destroy()
      {
        detail::variant_index_visit<TypeList>((destroy_visitor(*this)), discriminator);
      }
      template <class T>
      dynamic_storage& operator=(const T& val)
      {
        detail::variant_overload_resolve<TypeList>(val, (overload_assigner(*this)));
        return *this;
      }
      dynamic_storage& operator=(const dynamic_storage& other)
      {
        detail::variant_index_visit<TypeList>((copy_assign_visitor(other, *this)), other.discriminator);
        return *this;
      }
    };
  }

  // forward declarations
  template <std::size_t I, class TypeList, template <class> class Storage>
  typename type_traits::add_reference< typename variant_alternative<I, variant<TypeList, Storage> >::type>::type get(variant<TypeList, Storage>& v);
  template <std::size_t I, class TypeList, template <class> class Storage>
  typename type_traits::add_reference< typename type_traits::add_const< typename variant_alternative<I, variant<TypeList, Storage> >::type>::type>::type get(const variant<TypeList, Storage>& v);
  template <class T, class TypeList, template <class> class Storage>
  T& get(variant<TypeList, Storage>& v);
  template <class T, class TypeList, template <class> class Storage>
  const T& get(const variant<TypeList, Storage>& v);

  template <class TypeList, template <class> class Storage = variant_storage::local_storage>
  class variant
  {
    // friend declarations
    template <std::size_t I, class TypeList_, template <class> class Storage_>
    friend typename type_traits::add_reference< typename variant_alternative<I, variant<TypeList_, Storage_> >::type>::type get(variant<TypeList_, Storage_>& v);
    template <std::size_t I, class TypeList_, template <class> class Storage_>
    friend typename type_traits::add_reference<typename type_traits::add_const< typename variant_alternative<I, variant<TypeList, Storage> >::type>::type>::type get(const variant<TypeList_, Storage_>& v);
    template <class T, class TypeList_, template <class> class Storage_>
    friend T& get(variant<TypeList_, Storage_>& v);
    template <class T, class TypeList_, template <class> class Storage_>
    friend const T& get(const variant<TypeList_, Storage_>& v);
  private:
    typedef TypeList type_list;
    Storage<TypeList> storage;
    typedef typename type_traits::head<TypeList>::type head_type;
  public:
    std::size_t index() const MY_NOEXCEPT { return storage.get_discriminator(); }
    bool valueless_by_exception() const MY_NOEXCEPT{ return storage.get_discriminator() == variant_nops; }
  public:
    variant(): storage((head_type())) {}
    variant(const variant& other): storage(other.storage) {}
    template <class T>
    variant(const T& val): storage(val) {}
    template <std::size_t I, class T>
    variant(in_place_index_t<I>, const T& val): storage((in_place_index_t<I>()), val) {}
    template <class T, class U>
    variant(in_place_type_t<T>, const U& val): storage((T(val))) {}
    ~variant() MY_NOEXCEPT { storage.destroy(); }
    variant& operator=(const variant& other) { this->storage = other.storage; }
    template <class T>
    variant& operator=(const T& val) { this->storage = val; return *this; }
  };

  template <std::size_t I, class TypeList, template <class> class Storage>
  typename type_traits::add_reference< typename variant_alternative<I, variant<TypeList, Storage> >::type>::type get(variant<TypeList, Storage>& v)
  {
    typedef typename type_traits::get<TypeList, I>::type T;
    if (I != v.storage.get_discriminator()) throw bad_variant_access();
    return *v.storage.template get_as<T>();
  }
  template <std::size_t I, class TypeList, template <class> class Storage>
  typename type_traits::add_reference<typename type_traits::add_const< typename variant_alternative<I, variant<TypeList, Storage> >::type>::type>::type get(const variant<TypeList, Storage>& v)
  {
    typedef typename type_traits::get<TypeList, I>::type T;
    if (I != v.storage.get_discriminator()) throw bad_variant_access();
    return *v.storage.template get_as<T>();
  }
  template <class T, class TypeList, template <class> class Storage>
  T& get(variant<TypeList, Storage>& v)
  {
    if (!detail::is_match_type_index<T, TypeList>(v.index())) throw bad_variant_access();
    return *v.storage.template get_as<T>();
  }
  template <class T, class TypeList, template <class> class Storage>
  const T& get(const variant<TypeList, Storage>& v)
  {
    if (!detail::is_match_type_index<T, TypeList>(v.index())) throw bad_variant_access();
    return *v.storage.template get_as<T>();
  }

}

#endif

