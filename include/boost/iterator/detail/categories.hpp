// (C) Copyright Thomas Witt 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_ITERATOR_DETAIL_CATEGORIES_HPP
#define BOOST_ITERATOR_DETAIL_CATEGORIES_HPP

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool_c.hpp>
#include <boost/mpl/logical/or.hpp>
#include <iterator>

#if BOOST_WORKAROUND(__MWERKS__, <=0x2407)
#  define BOOST_NO_IS_CONVERTIBLE // "Convertible does not provide enough/is not working"
#endif

namespace boost {

  // Return Type Categories
  struct readable_iterator_tag { };
  struct writable_iterator_tag { };
  struct swappable_iterator_tag : 
    virtual public readable_iterator_tag, // Not sure about this change -JGS
    virtual public writable_iterator_tag { };
  struct constant_lvalue_iterator_tag : 
    virtual public readable_iterator_tag { };
  struct mutable_lvalue_iterator_tag : 
    virtual public swappable_iterator_tag,
    virtual public constant_lvalue_iterator_tag { };

  // Traversal Categories
  struct input_traversal_tag { };
  struct output_traversal_tag { };
  struct forward_traversal_tag : virtual public input_traversal_tag, 
    virtual public output_traversal_tag { };
  struct bidirectional_traversal_tag : virtual public forward_traversal_tag { };
  struct random_access_traversal_tag : virtual public bidirectional_traversal_tag { };

  struct error_iterator_tag { };

  namespace detail
  {
    //
    // Tag detection meta functions
    //
    // Needed to work with compilers
    // without working is_convertible
    //

#ifndef BOOST_NO_IS_CONVERTIBLE
    template <class Category>
    struct is_input_iterator :
      is_convertible<Category*, std::input_iterator_tag*>
    {
    };

    template <class Category>
    struct is_output_iterator :
      is_convertible<Category*, std::output_iterator_tag*>
    {
    };

    template <class Category>
    struct is_forward_iterator :
      is_convertible<Category*, std::forward_iterator_tag*>
    {
    };

    template <class Category>
    struct is_bidirectional_iterator :
      is_convertible<Category*, std::bidirectional_iterator_tag*>
    {
    };

    template <class Category>
    struct is_random_access_iterator :
      is_convertible<Category*, std::random_access_iterator_tag*>
    {
    };

    template <class Category>
    struct is_readable_iterator :
      is_convertible<Category*, readable_iterator_tag*>
    { };

    template <class Category>
    struct is_writable_iterator :
      is_convertible<Category*, writable_iterator_tag*>
    { };

    template <class Category>
    struct is_swappable_iterator :
      is_convertible<Category*, swappable_iterator_tag*>
    { };

    template <class Category>
    struct is_constant_lvalue_iterator :
      is_convertible<Category*, constant_lvalue_iterator_tag*>
    { };

    template <class Category>
    struct is_mutable_lvalue_iterator :
      is_convertible<Category*, mutable_lvalue_iterator_tag*>
    { };

    template <class Category>
    struct is_input_traversal_iterator :
      is_convertible<Category*, input_traversal_tag*>
    { };

    template <class Category>
    struct is_output_traversal_iterator :
      is_convertible<Category*, output_traversal_tag*>
    { };

    template <class Category>
    struct is_forward_traversal_iterator :
      is_convertible<Category*, forward_traversal_tag*>
    { };

    template <class Category>
    struct is_bidirectional_traversal_iterator :
      is_convertible<Category*, bidirectional_traversal_tag*>
    { };

    template <class Category>
    struct is_random_access_traversal_iterator :
      is_convertible<Category*, random_access_traversal_tag*>
    { };

#else
    //
    // We cannot detect the iterator category for custom
    // tag types. For custom tag types false_c is returned
    //
    // As a result the std iterator category detection functions
    // can't detect the iterator category of the new iterator_tag template.
    //

    //
    // std iterator categories
    //

    template <class Category>
    struct is_random_access_iterator :
      mpl::false_c
    {};

    template <>
    struct is_random_access_iterator<std::random_access_iterator_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_bidirectional_iterator :
      is_random_access_iterator< Category >
    {};

    template <>
    struct is_bidirectional_iterator<std::bidirectional_iterator_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_forward_iterator :
      is_bidirectional_iterator< Category >
    {};

    template <>
    struct is_forward_iterator<std::forward_iterator_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_output_iterator :
      is_forward_iterator< Category >
    {};

    template <>
    struct is_output_iterator<std::output_iterator_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_input_iterator :
      is_forward_iterator< Category >
    {};

    template <>
    struct is_input_iterator<std::input_iterator_tag> :
      mpl::true_c
    {};

    //
    // Return type
    //

    template <class Category>
    struct is_mutable_lvalue_iterator :
      mpl::false_c
    {};

    template <>
    struct is_mutable_lvalue_iterator<mutable_lvalue_iterator_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_constant_lvalue_iterator :
      is_mutable_lvalue_iterator<Category>
    {};

    template <>
    struct is_constant_lvalue_iterator<constant_lvalue_iterator_tag> :
      mpl::true_c 
    {};

    template <class Category>
    struct is_swappable_iterator :
      is_constant_lvalue_iterator<Category>
    {};

    template <>
    struct is_swappable_iterator<swappable_iterator_tag> :
      mpl::true_c 
    {};

    template <class Category>
    struct is_readable_iterator :
      mpl::logical_or< is_swappable_iterator<Category>,
                       is_constant_lvalue_iterator<Category> >
    {};

    template <>
    struct is_readable_iterator<readable_iterator_tag> :
      mpl::true_c 
    {};

    template <class Category>
    struct is_writable_iterator :
      is_swappable_iterator<Category>
    {};

    template <>
    struct is_writable_iterator<writable_iterator_tag> :
      mpl::true_c 
    {};

    //
    // Traversal
    //

    template <class Category>
    struct is_random_access_traversal_iterator :
      mpl::false_c
    {};

    template <>
    struct is_random_access_traversal_iterator<random_access_traversal_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_bidirectional_traversal_iterator :
      is_random_access_traversal_iterator< Category >
    {};

    template <>
    struct is_bidirectional_traversal_iterator<bidirectional_traversal_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_forward_traversal_iterator :
      is_bidirectional_traversal_iterator< Category >
    {};

    template <>
    struct is_forward_traversal_iterator<forward_traversal_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_input_traversal_iterator :
      is_forward_traversal_iterator< Category >
    {};

    template <>
    struct is_input_traversal_iterator<input_traversal_tag> :
      mpl::true_c
    {};

    template <class Category>
    struct is_output_traversal_iterator :
      is_forward_traversal_iterator< Category >
    {};

    template <>
    struct is_output_traversal_iterator<output_traversal_tag> :
      mpl::true_c
    {};

#endif

  } // namespace detail

} // namespace boost

#ifdef BOOST_NO_IS_CONVERTIBLE
#  undef BOOST_NO_IS_CONVERTIBLE
#endif

#endif // BOOST_ITERATOR_DETAIL_CATEGORIES_HPP
