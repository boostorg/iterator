// (C) Copyright Jeremy Siek 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_ITERATOR_CONCEPTS_HPP
#define BOOST_ITERATOR_CONCEPTS_HPP

//  Revision History
//  22 Nov 2002 Thomas Witt
//       Added interoperable concept.

#include <boost/concept_check.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/static_assert.hpp>

// Use boost::detail::iterator_traits to work around some MSVC/Dinkumware problems.
#include <boost/detail/iterator.hpp>

namespace boost_concepts {
  // Used a different namespace here (instead of "boost") so that the
  // concept descriptions do not take for granted the names in
  // namespace boost.

  // We use this in place of STATIC_ASSERT((is_convertible<...>))
  // because some compilers (CWPro7.x) can't detect convertibility.
  //
  // Of course, that just gets us a different error at the moment with
  // some tests, since new iterator category deduction still depends
  // on convertibility detection. We might need some specializations
  // to support this compiler.
  template <class Target, class Source>
  struct static_assert_base_and_derived
  {
      static_assert_base_and_derived(Target* x = (Source*)0) {}
  };

  //===========================================================================
  // Iterator Access Concepts

  template <typename Iterator>
  class ReadableIteratorConcept {
  public:
    typedef typename boost::detail::iterator_traits<Iterator>::value_type value_type;
    typedef typename boost::detail::iterator_traits<Iterator>::reference reference;
    typedef typename boost::return_category<Iterator>::type return_category;

    void constraints() {
      boost::function_requires< boost::SGIAssignableConcept<Iterator> >();
      boost::function_requires< boost::EqualityComparableConcept<Iterator> >();
      boost::function_requires< 
        boost::DefaultConstructibleConcept<Iterator> >();

      static_assert_base_and_derived<boost::readable_iterator_tag, return_category>();
                          
      reference r = *i; // or perhaps read(x)
      value_type v(r);
      boost::ignore_unused_variable_warning(v);
    }
    Iterator i;
  };
  
  template <typename Iterator, typename ValueType>
  class WritableIteratorConcept {
  public:
    typedef typename boost::return_category<Iterator>::type return_category;

    void constraints() {
      boost::function_requires< boost::SGIAssignableConcept<Iterator> >();
      boost::function_requires< boost::EqualityComparableConcept<Iterator> >();
      boost::function_requires< 
        boost::DefaultConstructibleConcept<Iterator> >();
      
      static_assert_base_and_derived<boost::writable_iterator_tag, return_category>();

      *i = v; // a good alternative could be something like write(x, v)
    }
    ValueType v;
    Iterator i;
  };
  
  template <typename Iterator>
  class ConstantLvalueIteratorConcept {
  public:
    typedef typename boost::detail::iterator_traits<Iterator>::value_type value_type;
    typedef typename boost::detail::iterator_traits<Iterator>::reference reference;
    typedef typename boost::return_category<Iterator>::type return_category;

    void constraints() {
      boost::function_requires< ReadableIteratorConcept<Iterator> >();

      static_assert_base_and_derived<boost::constant_lvalue_iterator_tag, return_category>();

      BOOST_STATIC_ASSERT((boost::is_same<reference, 
                           const value_type&>::value));

      reference v = *i;
      boost::ignore_unused_variable_warning(v);
    }
    Iterator i;
  };

  template <typename Iterator>
  class MutableLvalueIteratorConcept {
  public:
    typedef typename boost::detail::iterator_traits<Iterator>::value_type value_type;
    typedef typename boost::detail::iterator_traits<Iterator>::reference reference;
    typedef typename boost::return_category<Iterator>::type return_category;

    void constraints() {
      boost::function_requires< ReadableIteratorConcept<Iterator> >();
      boost::function_requires< 
        WritableIteratorConcept<Iterator, value_type> >();
      
      static_assert_base_and_derived<boost::mutable_lvalue_iterator_tag, return_category>();

      BOOST_STATIC_ASSERT((boost::is_same<reference, value_type&>::value));

      reference v = *i;
      boost::ignore_unused_variable_warning(v);
    }
    Iterator i;
  };
  
  //===========================================================================
  // Iterator Traversal Concepts

  template <typename Iterator>
  class ForwardTraversalConcept {
  public:
    typedef typename boost::traversal_category<Iterator>::type traversal_category;

    void constraints() {
      boost::function_requires< boost::SGIAssignableConcept<Iterator> >();
      boost::function_requires< boost::EqualityComparableConcept<Iterator> >();
      boost::function_requires< 
        boost::DefaultConstructibleConcept<Iterator> >();

      static_assert_base_and_derived<boost::forward_traversal_tag, traversal_category>();

      ++i;
      (void)i++;
    }
    Iterator i;
  };
  
  template <typename Iterator>
  class BidirectionalTraversalConcept {
  public:
    typedef typename boost::traversal_category<Iterator>::type traversal_category;

    void constraints() {
      boost::function_requires< ForwardTraversalConcept<Iterator> >();
      
      static_assert_base_and_derived<boost::bidirectional_traversal_tag, traversal_category>();

      --i;
      (void)i--;
    }
    Iterator i;
  };

  template <typename Iterator>
  class RandomAccessTraversalConcept {
  public:
    typedef typename boost::traversal_category<Iterator>::type traversal_category;
    typedef typename boost::detail::iterator_traits<Iterator>::difference_type
      difference_type;

    void constraints() {
      boost::function_requires< BidirectionalTraversalConcept<Iterator> >();

      static_assert_base_and_derived<boost::random_access_traversal_tag, traversal_category>();

      
      i += n;
      i = i + n;
      i = n + i;
      i -= n;
      i = i - n;
      n = i - j;
    }
    difference_type n;
    Iterator i, j;
  };

  //===========================================================================
  // Iterator Interoperability Concept

  namespace detail {

    template <typename TraversalTag>
    struct Operations;
    
    template <>
    struct Operations<boost::input_traversal_tag>
    {
      template <typename Iterator1,
                typename Iterator2>
      static void constraints(Iterator1 const& i1, Iterator2 const& i2)
      {
        i1 == i2;
        i1 != i2;

        i2 == i1;
        i2 != i1;
      }
    };

    template <>
    struct Operations<boost::output_traversal_tag>
    {
      template <typename Iterator1,
                typename Iterator2>
      static void constraints(Iterator1 const& i1, Iterator2 const& i2)
      {
        Operations<boost::input_traversal_tag>::constraints(i1, i2);
      }
    };

    template <>
    struct Operations<boost::forward_traversal_tag>
    {
      template <typename Iterator1,
                typename Iterator2>
      static void constraints(Iterator1 const& i1, Iterator2 const& i2)
      {
        Operations<boost::input_traversal_tag>::constraints(i1, i2);
      }
    };

    template <>
    struct Operations<boost::bidirectional_traversal_tag>
    {
      template <typename Iterator1,
                typename Iterator2>
      static void constraints(Iterator1 const& i1, Iterator2 const& i2)
      {
        Operations<boost::forward_traversal_tag>::constraints(i1, i2);
      }
    };

    template <>
    struct Operations<boost::random_access_traversal_tag>
    {
      template <typename Iterator1,
                typename Iterator2>
      static void constraints(Iterator1 const& i1, Iterator2 const& i2)
      {
        Operations<boost::bidirectional_traversal_tag>::constraints(i1, i2);

        i1 <  i2;
        i1 <= i2;
        i1 >  i2;
        i1 >= i2;
        i1 -  i2;

        i2 <  i1;
        i2 <= i1;
        i2 >  i1;
        i2 >= i1;
        i2 -  i1;
      }
    };

  };

  template <typename Iterator,
            typename ConstIterator>
  class InteroperableConcept
  {
  public:
    typedef typename boost::traversal_category<Iterator>::type traversal_category;
    typedef typename boost::detail::iterator_traits<Iterator>::difference_type
      difference_type;

    typedef typename boost::traversal_category<ConstIterator>::type
      const_traversal_category;
    typedef typename boost::detail::iterator_traits<ConstIterator>::difference_type
      const_difference_type;

    void constraints() {
      BOOST_STATIC_ASSERT((boost::is_same< difference_type,
                                           const_difference_type>::value));
      BOOST_STATIC_ASSERT((boost::is_same< traversal_category,
                                           const_traversal_category>::value));

      detail::Operations<traversal_category>::constraints(i, ci);

      ci = i;

    }
    Iterator      i;
    ConstIterator ci;
  };

} // namespace boost_concepts


#endif // BOOST_ITERATOR_CONCEPTS_HPP
