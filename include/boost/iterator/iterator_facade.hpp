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
#include <boost/iterator/interoperable.hpp>
#include <boost/iterator/detail/enable_if.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/iterator/detail/config_def.hpp>

namespace boost
{

  namespace detail
  {

    //
    // enable if for use in operator implementation.
    //
    // enable_if_interoperable falls back to always enabled for compilers
    // that don't support enable_if or is_convertible. 
    //
    template <class Facade1,
              class Facade2,
              class Return>
    struct enable_if_interoperable :
      ::boost::detail::enable_if< is_convertible< Facade1, Facade2 >
                                  , Return >
    {
# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
        typedef Return type;
# endif 
    };


    //
    // Type generator.
    // Generates the corresponding std::iterator specialization
    // from the given iterator traits type
    //
    template <class Traits>
    struct std_iterator_from_traits
         : iterator<
             typename Traits::iterator_category
           , typename Traits::value_type
           , typename Traits::difference_type
           , typename Traits::pointer
           , typename Traits::reference
        >
    {
    };

    
  } // namespace detail


  //
  // Helper class for granting access to the iterator core interface.
  //
  // The simple core interface is used by iterator_facade. The core
  // interface of a user/library defined iterator type should not be made public
  // so that it does not clutter the public interface. Instead iterator_core_access
  // should be made friend so that iterator_facade can access the core
  // interface through iterator_core_access.
  //
  struct iterator_core_access
  {
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
    static bool equal(Facade1 const& f1, Facade2 const& f2, Facade1* = 0, Facade2* = 0)
    {
      return f1.equal(f2);
    }

    template <class Facade>
    static void advance(Facade& f, typename Facade::difference_type n)
    {
      f.advance(n);
    }

    template <class Facade1, class Facade2>
    static typename Facade1::difference_type distance_to(Facade1 const& f1,
                                                         Facade2 const& f2,
                                                         Facade1* = 0,
                                                         Facade2* = 0)
    {
      return f1.distance_to(f2);
    }

  private:
    // objects of this class are useless
    iterator_core_access(); //undefined
  };

  //
  //
  // iterator_facade applies iterator_traits_adaptor to it's traits argument.
  // The net effect is that iterator_facade is derived from std::iterator. This
  // is important for standard library interoperability of iterator types on some
  // (broken) implementations. 
  //
  template <
      class Derived
    , class Traits
  >
  class iterator_facade
      : public detail::std_iterator_from_traits<Traits>
  {
      typedef detail::std_iterator_from_traits<Traits> super_t;

   public:
      //
      // CRT interface. There is no simple way to remove this
      // from the public interface without template friends
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


      typedef typename super_t::reference reference;
      typedef typename super_t::difference_type difference_type;
      typedef typename super_t::pointer pointer;

      reference operator*() const
      {
          return iterator_core_access::dereference(this->derived());
      }

      // Needs eventual help for input iterators
      pointer operator->() const
      {
          return &iterator_core_access::dereference(this->derived());
      }
        
      reference operator[](difference_type n) const
      {
          return *(*this + n);
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
  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator==(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return iterator_core_access::equal(lhs.derived(),
                                       rhs.derived());
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator!=(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return !iterator_core_access::equal(lhs.derived(),
                                        rhs.derived());
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator<(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) > 0;
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator>(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) < 0;
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator<=(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) >= 0;
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           bool>::type
  operator>=(iterator_facade<Derived1, Traits1> const& lhs,
             iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) <= 0;
  }

  template <class Derived,
            class Traits>
  inline
  Derived operator+(iterator_facade<Derived, Traits> const& i,
                    typename Traits::difference_type n)
  {
    Derived tmp(i.derived());
    return tmp += n;
  }

  template <class Derived,
            class Traits>
  inline
  Derived operator+(typename Traits::difference_type n,
                    iterator_facade<Derived, Traits> const& i)
  {
    Derived tmp(i.derived());
    return tmp += n;
  }

  template <class Derived1,
            class Traits1,
            class Derived2,
            class Traits2>
  inline
  typename detail::enable_if_interoperable<Derived1,
                                           Derived2,
                                           typename Traits1::difference_type>::type
  operator-(iterator_facade<Derived1, Traits1> const& lhs,
            iterator_facade<Derived2, Traits2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((is_interoperable< Derived1, Derived2 >::value));

    BOOST_STATIC_ASSERT((is_same<BOOST_ARG_DEP_TYPENAME Traits1::difference_type,
                                 BOOST_ARG_DEP_TYPENAME Traits2::difference_type>::value));

    return iterator_core_access::distance_to(rhs.derived(),
                                             lhs.derived());
  }

} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_ITERATOR_FACADE_23022003THW_HPP
