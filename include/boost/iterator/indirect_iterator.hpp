// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef BOOST_INDIRECT_ITERATOR_23022003THW_HPP
#define BOOST_INDIRECT_ITERATOR_23022003THW_HPP

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include <boost/iterator/detail/config_def.hpp>

#ifdef BOOST_NO_MPL_AUX_HAS_XXX
# include <boost/shared_ptr.hpp>
# include <boost/scoped_ptr.hpp>
# include <boost/mpl/bool.hpp>
# include <memory>
#endif 

namespace boost
{

  namespace detail
  {
    struct unspecified {};

    //
    // Detection for whether a type has a nested `element_type'
    // typedef. Used to detect smart pointers. For compilers not
    // supporting mpl's has_xxx, we supply specializations. However, we
    // really ought to have a specializable is_pointer template which
    // can be used instead with something like
    // boost/python/pointee.hpp to find the value_type.
    //
# if !defined BOOST_NO_MPL_AUX_HAS_XXX
    namespace aux
    {
      BOOST_MPL_HAS_XXX_TRAIT_DEF(element_type)
    }

    template <class T>
    struct has_element_type
    {
      typedef typename mpl::if_<
          is_class<T>
        , aux::has_element_type<T>
        , mpl::false_
          >::type type;
    };
# else
    template <class T>
    struct has_element_type
        : mpl::false_c {};
    
    template <class T>
    struct has_element_type<boost::shared_ptr<T> >
        : mpl::true_c {};
    
    template <class T>
    struct has_element_type<boost::scoped_ptr<T> >
        : mpl::true_c {};
    
    template <class T>
    struct has_element_type<std::auto_ptr<T> >
        : mpl::true_c {};
# endif 
  
    // Metafunction returning the nested element_type typedef
    template <class T>
    struct smart_pointer_traits
    {
      typedef typename remove_const<
        typename T::element_type
      >::type value_type;

      typedef typename T::element_type& reference;
      typedef typename T::element_type* pointer;
    };

    // If the Value parameter is unspecified, we use this metafunction
    // to deduce the default types
    template <class Iter>
    struct indirect_defaults
      : mpl::if_<
            has_element_type<typename iterator_traits<Iter>::value_type>
          , smart_pointer_traits<typename iterator_traits<Iter>::value_type>
          , iterator_traits<typename iterator_traits<Iter>::value_type>
        >::type
    {
      typedef typename iterator_traits<Iter>::iterator_category iterator_category;
      typedef typename iterator_traits<Iter>::difference_type difference_type;
    };

    // The traits to use for indirect iterator, by default.  Whatever
    // is supplied gets passed through traits_iterator<...> so that it
    // is ultimately derived from boost::iterator<...>
    template <class Base, class Traits>
    struct indirect_traits
      : mpl::if_<
            is_same<Traits,unspecified>
            , indirect_defaults<Base>
            , Traits
           >::type
    {
    };
  } // namespace detail

  template <class Iterator, class Traits = detail::unspecified>
  class indirect_iterator :
    public iterator_adaptor<
        indirect_iterator<Iterator, Traits>
      , Iterator
      , detail::indirect_traits<Iterator,Traits> >
  {
    typedef iterator_adaptor<
          indirect_iterator<Iterator, Traits>
        , Iterator
        , detail::indirect_traits<Iterator,Traits>
    > super_t;

    friend class iterator_core_access;

  public:
    indirect_iterator() {}

    indirect_iterator(Iterator iter)
      : super_t(iter) {}

    template <class OtherIterator,
              class OtherTraits>
    indirect_iterator(
        indirect_iterator<OtherIterator, OtherTraits> const& y
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
        )
      : super_t(y.base())
      {}
      
  private:    
    typename super_t::reference dereference() const { return **this->base(); }

  };

  template <class Iter>
  inline
  indirect_iterator<Iter> make_indirect_iterator(Iter x)
  {
    return indirect_iterator<Iter>(x);
  }

  template <class Traits, class Iter>
  inline
  indirect_iterator<Iter,Traits> make_indirect_iterator(Iter x, Traits* = 0)
  {
    return indirect_iterator<Iter, Traits>(x);
  }

} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_INDIRECT_ITERATOR_23022003THW_HPP
