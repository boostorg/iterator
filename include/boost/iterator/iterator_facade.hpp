// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef BOOST_ITERATOR_FACADE_23022003THW_HPP
#define BOOST_ITERATOR_FACADE_23022003THW_HPP

#include <boost/static_assert.hpp>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/interoperable.hpp>
#include <boost/iterator/detail/enable_if.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/iterator/iterator_traits.hpp>

#include <boost/mpl/apply_if.hpp>
#include <boost/mpl/or.hpp>

#include <boost/iterator/detail/config_def.hpp> // this goes last

namespace boost
{

  struct use_default;
  
  namespace detail
  {

    //
    // enable if for use in operator implementation.
    //
    // enable_if_interoperable falls back to always enabled for compilers
    // that don't support enable_if or is_convertible. 
    //
    template <
        class Facade1
      , class Facade2
      , class Return
    >
    struct enable_if_interoperable
#ifndef BOOST_NO_STRICT_ITERATOR_INTEROPERABILITY
      : ::boost::detail::enable_if<
           mpl::or_<
               is_convertible<Facade1, Facade2>
             , is_convertible<Facade2, Facade1>
           >
         , Return
        >
#endif 
    {
#ifdef BOOST_NO_STRICT_ITERATOR_INTEROPERABILITY
        typedef Return type;
#endif 
    };


    template<class Value, class AccessCategory>
    struct const_qualified :
      mpl::if_< is_tag< writable_iterator_tag, AccessCategory >,
                Value,
                Value const >
    {};

    //
    // Type generator.
    // Generates the corresponding std::iterator specialization
    // from the given iterator traits type
    //
    template <class Value, class AccessCategory, class TraversalCategory, class Reference, class Difference>
    struct iterator_facade_base
    {
        typedef iterator<
            iterator_tag<AccessCategory, TraversalCategory>

          , typename remove_cv<Value>::type

          , Difference

          , typename const_qualified<Value, AccessCategory>::type*

          // The use_default support is needed for iterator_adaptor.
          // For practical reasons iterator_adaptor needs to specify
          // a fixed number of template arguments of iterator_facade.
          // So use_default is its way to say: "What I really mean
          // is your default parameter".
          , typename mpl::if_<
                is_same<Reference, use_default>
              , typename const_qualified<Value, AccessCategory>::type&
              , Reference
            >::type
        >
        type;
    };

    
    // operator->() needs special support for input iterators to strictly meet the
    // standard's requirements. If *i is not a reference type, we must still
    // produce a (constant) lvalue to which a pointer can be formed. We do that by
    // returning an instantiation of this special proxy class template.
    
    template <class T>
    struct operator_arrow_proxy
    {
        operator_arrow_proxy(T const* px) : m_value(*px) {}
        const T* operator->() const { return &m_value; }
        // This function is needed for MWCW and BCC, which won't call operator->
        // again automatically per 13.3.1.2 para 8
        operator const T*() const { return &m_value; }
        T m_value;
    };

    // A metafunction that gets the result type for operator->.  Also
    // has a static function make() which builds the result from a
    // Reference
    template <class Value, class Category, class Reference, class Pointer>
    struct operator_arrow_result
    {
        // CWPro8.3 won't accept "operator_arrow_result::type", and we
        // need that type below, so metafunction forwarding would be a
        // losing proposition here.
        typedef typename mpl::if_<
            is_tag<
                readable_lvalue_iterator_tag
              , typename access_category_tag<Category,Reference>::type
            >
          , Pointer
          , operator_arrow_proxy<Value>
        >::type type;
    
        static type make(Reference x)
        {
            return type(&x);
        }
    };
    
# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
    // Deal with ETI
    template<>
    struct operator_arrow_result<int, int, int, int>
    {
        typedef int type;
    };
# endif

    //
    // Facade is actually an iterator. We require Facade here
    // so that we do not have to go through iterator_traits
    // to access the traits
    //
    template <class Iterator>
    class operator_brackets_proxy
    {
        typedef typename Iterator::reference  reference;
        typedef typename Iterator::value_type value_type;

     public:
        operator_brackets_proxy(Iterator const& iter)
            : m_iter(iter)
        {}

        operator reference()
        {
            return *m_iter;
        }

        operator_brackets_proxy& operator=(value_type const& val)
        {
            *m_iter = val;
            return *this;
        }
      
     private:
        Iterator m_iter;
    };

    template <class Iterator, class ValueType, class Category, class Reference>
    struct operator_brackets_result
    {
        typedef typename access_category_tag<Category,Reference>::type access_category;
        
        typedef is_tag<writable_iterator_tag, access_category> use_proxy;

        typedef typename mpl::if_< 
            use_proxy
          , operator_brackets_proxy<Iterator>
          , ValueType 
        >::type type;
    };

    template <class Iterator>
    operator_brackets_proxy<Iterator> make_operator_brackets_result(Iterator const& iter, mpl::true_)
    {
        return operator_brackets_proxy<Iterator>(iter);
    }

    template <class Iterator>
    typename Iterator::value_type make_operator_brackets_result(Iterator const& iter, mpl::false_)
    {
      return *iter;
    }

  } // namespace detail


  // Macros which describe the declarations of binary operators
#  define BOOST_ITERATOR_FACADE_INTEROP_HEAD(prefix, op, result_type)       \
    template <                                                              \
        class Derived1, class V1, class AC1, class TC1, class R1, class D1              \
      , class Derived2, class V2, class AC2, class TC2, class R2, class D2              \
    >                                                                       \
    prefix typename detail::enable_if_interoperable<                        \
        Derived1, Derived2, result_type                                     \
    >::type                                                                 \
    operator op(                                                            \
        iterator_facade<Derived1, V1, AC1, TC1, R1, D1> const& lhs                \
      , iterator_facade<Derived2, V2, AC2, TC2, R2, D2> const& rhs)

#  define BOOST_ITERATOR_FACADE_PLUS_HEAD(prefix,args)                      \
    template <class Derived, class V, class AC, class TC, class R, class D>        \
    prefix Derived operator+ args

  //
  // Helper class for granting access to the iterator core interface.
  //
  // The simple core interface is used by iterator_facade. The core
  // interface of a user/library defined iterator type should not be made public
  // so that it does not clutter the public interface. Instead iterator_core_access
  // should be made friend so that iterator_facade can access the core
  // interface through iterator_core_access.
  //
  class iterator_core_access
  {
# if defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)                  \
    || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
      // Tasteless as this may seem, making all members public allows member templates
      // to work in the absence of member template friends.
   public:
# else
      
    template <class I, class V, class AC, class TC, class R, class D> friend class iterator_facade;

#  define BOOST_ITERATOR_FACADE_RELATION(op)     \
      BOOST_ITERATOR_FACADE_INTEROP_HEAD(friend,op, bool);

      BOOST_ITERATOR_FACADE_RELATION(==)
      BOOST_ITERATOR_FACADE_RELATION(!=)

      BOOST_ITERATOR_FACADE_RELATION(<)
      BOOST_ITERATOR_FACADE_RELATION(>)
      BOOST_ITERATOR_FACADE_RELATION(<=)
      BOOST_ITERATOR_FACADE_RELATION(>=)
# undef BOOST_ITERATOR_FACADE_RELATION

      BOOST_ITERATOR_FACADE_INTEROP_HEAD(
          friend, -, typename Derived1::difference_type)
      ;

      BOOST_ITERATOR_FACADE_PLUS_HEAD(
          friend                                
          , (iterator_facade<Derived, V, AC, TC, R, D> const&
           , typename Derived::difference_type)
      )
      ;

      BOOST_ITERATOR_FACADE_PLUS_HEAD(
          friend                           
        , (typename Derived::difference_type
           , iterator_facade<Derived, V, AC, TC, R, D> const&)
      )
      ;
      
#endif
      
      template <class Facade>
      static typename Facade::reference dereference(Facade const& f)
      {
          return f.dereference();
      }

      template <class Facade>
      static void increment(Facade& f)
      {
          f.increment();
      }

      template <class Facade>
      static void decrement(Facade& f)
      {
          f.decrement();
      }

      template <class Facade1, class Facade2>
      static bool equal(Facade1 const& f1, Facade2 const& f2)
      {
          return f1.equal(f2);
      }

      template <class Facade>
      static void advance(Facade& f, typename Facade::difference_type n)
      {
          f.advance(n);
      }

      template <class Facade1, class Facade2>
      static typename Facade1::difference_type distance_to(
                                                           Facade1 const& f1, Facade2 const& f2)
      {
          return f1.distance_to(f2);
      }

   private:
      // objects of this class are useless
      iterator_core_access(); //undefined
  };

  //
  //
  // iterator_facade applies iterator_traits_adaptor to its traits argument.
  // The net effect is that iterator_facade is derived from std::iterator. This
  // is important for standard library interoperability of iterator types on some
  // (broken) implementations. 
  //
  template <
      class Derived
    , class Value
    , class AccessCategory
    , class TraversalCategory
    , class Reference   = typename detail::const_qualified<Value, AccessCategory>::type&
    , class Difference  = std::ptrdiff_t
  >
  class iterator_facade
    : public detail::iterator_facade_base<Value, AccessCategory, TraversalCategory, Reference, Difference>::type
  {
   private:
      typedef typename
      detail::iterator_facade_base<Value, AccessCategory, TraversalCategory, Reference, Difference>::type
      super_t;

      //
      // CRT interface.
      //
      typedef Derived derived_t;
      
      Derived& derived()
      {
          return static_cast<Derived&>(*this);
      }

      Derived const& derived() const 
      {
          return static_cast<Derived const&>(*this);
      }

   public:

      typedef typename super_t::value_type value_type;
      typedef typename super_t::reference reference;
      typedef typename super_t::difference_type difference_type;
      typedef typename super_t::pointer pointer;
      typedef typename super_t::iterator_category iterator_category;

      reference operator*() const
      {
          return iterator_core_access::dereference(this->derived());
      }

      typename detail::operator_arrow_result<
          value_type
        , iterator_category
        , reference
        , pointer
      >::type
      operator->() const
      {
          return detail::operator_arrow_result<
              value_type
            , iterator_category
            , reference
            , pointer
          >::make(*this->derived());
      }
        
      typename detail::operator_brackets_result<Derived,value_type,iterator_category,reference>::type
      operator[](difference_type n) const
      {
          typedef typename
              detail::operator_brackets_result<Derived,value_type,iterator_category,reference>::use_proxy
          use_proxy;
          
          return detail::make_operator_brackets_result<Derived>(this->derived() + n, use_proxy());
      }

      Derived& operator++()
      {
          iterator_core_access::increment(this->derived());
          return this->derived();
      }

      Derived operator++(int)
      {
          Derived tmp(this->derived());
          ++*this;
          return tmp;
      }
  
      Derived& operator--()
      {
          iterator_core_access::decrement(this->derived());
          return this->derived();
      }
  
      Derived operator--(int)
      {
          Derived tmp(this->derived());
          --*this;
          return tmp;
      }
  
      Derived& operator+=(difference_type n)
      {
          iterator_core_access::advance(this->derived(), n);
          return this->derived();
      }
  
      Derived& operator-=(difference_type n)
      {
          iterator_core_access::advance(this->derived(), -n);
          return this->derived();
      }
  
      Derived operator-(difference_type x) const
      {
          Derived result(this->derived());
          return result -= x;
      }
  };

  //
  // Operator implementation. The library supplied operators 
  // enables the user to provide fully interoperable constant/mutable
  // iterator types. I.e. the library provides all operators
  // for all mutable/constant iterator combinations.
  //
  // Note though that this kind of interoperability for constant/mutable
  // iterators is not required by the standard for container iterators.
  // All the standard asks for is a conversion mutable -> constant.
  // Most standard library implementations nowadays provide fully interoperable
  // iterator implementations, but there are still heavily used implementations 
  // that do not provide them. (Actually it's even worse, they do not provide 
  // them for only a few iterators.)
  //
  // ?? Maybe a BOOST_ITERATOR_NO_FULL_INTEROPERABILITY macro should
  //    enable the user to turn off mixed type operators 
  //
  // The library takes care to provide only the right operator overloads.
  // I.e.
  //
  // bool operator==(Iterator,      Iterator);
  // bool operator==(ConstIterator, Iterator);
  // bool operator==(Iterator,      ConstIterator);
  // bool operator==(ConstIterator, ConstIterator);
  //
  //   ...
  //
  // In order to do so it uses c++ idioms that are not yet widely supported
  // by current compiler releases. The library is designed to degrade gracefully
  // in the face of compiler deficiencies. In general compiler
  // deficiencies result in less strict error checking and more obscure
  // error messages, functionality is not affected.
  //
  // For full operation compiler support for "Substitution Failure Is Not An Error" 
  // (aka. enable_if) and boost::is_convertible is required.
  //
  // The following problems occur if support is lacking.
  //
  // Pseudo code
  //
  // ---------------
  // AdaptorA<Iterator1> a1;
  // AdaptorA<Iterator2> a2;
  //
  // // This will result in a no such overload error in full operation
  // // If enable_if or is_convertible is not supported
  // // The instantiation will fail with an error hopefully indicating that
  // // there is no operator== for Iterator1, Iterator2
  // // The same will happen if no enable_if is used to remove
  // // false overloads from the templated conversion constructor
  // // of AdaptorA.
  //
  // a1 == a2; 
  // ----------------
  //
  // AdaptorA<Iterator> a;
  // AdaptorB<Iterator> b;
  //
  // // This will result in a no such overload error in full operation
  // // If enable_if is not supported the static assert used
  // // in the operator implementation will fail.
  // // This will accidently work if is_convertible is not supported.
  //
  // a == b;
  // ----------------
  //

# define BOOST_ITERATOR_FACADE_INTEROP(op, result_type, condition, return_prefix, base_op)  \
  BOOST_ITERATOR_FACADE_INTEROP_HEAD(inline, op, result_type)                               \
  {                                                                                         \
      /* For those compilers that do not support enable_if */                               \
      BOOST_STATIC_ASSERT((                                                                 \
          is_interoperable< Derived1, Derived2 >::value                                     \
          && condition                                                                      \
      ));                                                                                   \
      return_prefix iterator_core_access::base_op(                                          \
          static_cast<Derived2 const&>(rhs), static_cast<Derived1 const&>(lhs));            \
  } 

# define BOOST_ITERATOR_FACADE_RELATION(op, return_prefix, base_op) \
  BOOST_ITERATOR_FACADE_INTEROP(                                    \
      op                                                            \
    , bool                                                          \
    , true                                                          \
    , return_prefix                                                 \
    , base_op                                                       \
  )

  BOOST_ITERATOR_FACADE_RELATION(==, return, equal)
  BOOST_ITERATOR_FACADE_RELATION(!=, return !, equal)

  BOOST_ITERATOR_FACADE_RELATION(<, return 0 >, distance_to)
  BOOST_ITERATOR_FACADE_RELATION(>, return 0 <, distance_to)
  BOOST_ITERATOR_FACADE_RELATION(<=, return 0 >=, distance_to)
  BOOST_ITERATOR_FACADE_RELATION(>=, return 0 <=, distance_to)
# undef BOOST_ITERATOR_FACADE_RELATION

  // operator- requires an additional part in the static assertion
  BOOST_ITERATOR_FACADE_INTEROP(
      -
    , typename Derived1::difference_type
    , (is_same<
           BOOST_DEDUCED_TYPENAME Derived1::difference_type
         , BOOST_DEDUCED_TYPENAME Derived2::difference_type
       >::value)
    , return
    , distance_to )
# undef BOOST_ITERATOR_FACADE_INTEROP
# undef BOOST_ITERATOR_FACADE_INTEROP_HEAD
      
# define BOOST_ITERATOR_FACADE_PLUS(args)           \
  BOOST_ITERATOR_FACADE_PLUS_HEAD(inline, args)     \
  {                                                 \
      Derived tmp(static_cast<Derived const&>(i));  \
      return tmp += n;                              \
  }

BOOST_ITERATOR_FACADE_PLUS((
  iterator_facade<Derived, V, AC, TC, R, D> const& i
  , typename Derived::difference_type n
))

BOOST_ITERATOR_FACADE_PLUS((
    typename Derived::difference_type n
    , iterator_facade<Derived, V, AC, TC, R, D> const& i
))
# undef BOOST_ITERATOR_FACADE_PLUS
# undef BOOST_ITERATOR_FACADE_PLUS_HEAD
    
} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_ITERATOR_FACADE_23022003THW_HPP
