// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef BOOST_ITERATOR_ADAPTOR_23022003THW_HPP
#define BOOST_ITERATOR_ADAPTOR_23022003THW_HPP

#include <boost/static_assert.hpp>
#include <boost/iterator.hpp>
#include <boost/detail/iterator.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/detail/enable_if.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/iterator/detail/config_def.hpp>

namespace boost
{
  
  namespace detail
  {
    template <class Traits, class Other>
    struct same_category_and_difference
      : mpl::and_<
          is_same<
              typename Traits::iterator_category
            , typename Other::iterator_category
          >
          , is_same<
              typename Traits::iterator_category
            , typename Other::iterator_category
          >
        >
    {};


    // 
    // Result type used in enable_if_convertible meta function.
    // This can be an incomplete type, as only pointers to 
    // enable_if_convertible< ... >::type are used.
    // We could have used void for this, but conversion to
    // void* is just to easy.
    //
    struct enable_type;
  }


  //
  // enable_if for use in adapted iterators constructors.
  //
  // In order to provide interoperability between adapted constant and
  // mutable iterators, adapted iterators will usually provide templated
  // conversion constructors of the following form
  //
  // template <class BaseIterator>
  // class adapted_iterator :
  //   public iterator_adaptor< adapted_iterator<Iterator>, Iterator >
  // {
  // public:
  //   
  //   ...
  //
  //   template <class OtherIterator>
  //   adapted_iterator(OtherIterator const& it,
  //                    typename enable_if_convertible<OtherIterator, Iterator>::type* = 0);
  //
  //   ...
  // };
  //
  // enable_if_convertible is used to remove those overloads from the overload
  // set that cannot be instantiated. For all practical purposes only overloads
  // for constant/mutable interaction will remain. This has the advantage that
  // meta functions like boost::is_convertible do not return false positives,
  // as they can only look at the signature of the conversion constructor
  // and not at the actual instantiation.
  //
  // enable_if_interoperable can be safely used in user code. It falls back to
  // always enabled for compilers that don't support enable_if or is_convertible. 
  // There is no need for compiler specific workarounds in user code. 
  //
  // The operators implementation relies on boost::is_convertible not returning
  // false positives for user/library defined iterator types. See comments
  // on operator implementation for consequences.
  //
  template<
      typename From
    , typename To>
  struct enable_if_convertible
  {
      // Borland 551 and vc6 have a problem with the use of base class
      // forwarding in this template, so  we write it all out here
# if defined(BOOST_NO_IS_CONVERTIBLE) || defined(BOOST_NO_SFINAE)
      typedef detail::enable_type type;
# else
      typedef typename detail::enable_if<
#  if BOOST_WORKAROUND(_MSC_FULL_VER, BOOST_TESTED_AT(13102292) && BOOST_MSVC > 1300)
          // For some reason vc7.1 needs us to "cut off" instantiation
          // of is_convertible in the case where From == To.
          mpl::or_<is_same<From,To>, is_convertible<From, To> >
#  else 
          ::boost::is_convertible<From, To>
#  endif 
      ,detail::enable_type >::type type;
# endif 
  };

  //
  // iterator_traits_adaptor can be used to create new iterator traits by adapting
  // the traits of a given iterator type. Together with iterator_adaptor it simplifies
  // the creation of adapted iterator types. Therefore the ordering the template
  // argument ordering is different from the std::iterator template, so that default 
  // arguments can be used effectivly.
  //
  template <class Iterator,
            class ValueType        = typename detail::iterator_traits<Iterator>::value_type,
            class Reference        = ValueType&,
            class Pointer          = ValueType*,
            class IteratorCategory = typename detail::iterator_traits<Iterator>::iterator_category,
            class DifferenceType   = typename detail::iterator_traits<Iterator>::difference_type >
  struct iterator_traits_adaptor
    : iterator<IteratorCategory,
               ValueType,
               DifferenceType,
               Pointer,
               Reference>
  {
  };

  //
  //
  //
  template <
      class Derived
      , class Iterator
      , class Traits = detail::iterator_traits<Iterator>
  >
  class iterator_adaptor
      : public iterator_facade<Derived,Traits>
  {
    friend class iterator_core_access;

  public:
    iterator_adaptor() {}

    explicit iterator_adaptor(Iterator iter)
      : m_iterator(iter)
    {
    }

    Iterator base() const { return m_iterator; }

  protected:
    // Core iterator interface for iterator_facade
    // 

    typename Traits::reference dereference() const { return *m_iterator; }

    template <
        class OtherDerived, class OtherIterator, class OtherTraits
    >   
    bool equal(iterator_adaptor<OtherDerived,OtherIterator,OtherTraits> const& x) const
    {
        BOOST_STATIC_ASSERT(
            (detail::same_category_and_difference<Traits,OtherTraits>::value)
            );
        return m_iterator == x.base();
    }
  
    void advance(typename Traits::difference_type n)
    {
        m_iterator += n;
    }
  
    void increment() { ++m_iterator; }
    void decrement() { --m_iterator; }

    template <class OtherDerived, class OtherIterator, class OtherTraits>   
    typename Traits::difference_type distance_to(
        iterator_adaptor<OtherDerived, OtherIterator, OtherTraits> const& y) const
    {
        BOOST_STATIC_ASSERT(
            (detail::same_category_and_difference<Traits,OtherTraits>::value)
            );
        return y.base() - m_iterator;
    }

   private: // data members
      Iterator m_iterator;

  };

} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_ITERATOR_ADAPTOR_23022003THW_HPP
