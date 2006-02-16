// (C) Copyright David Abrahams and Thomas Becker 2000. Permission to
// copy, use, modify, sell and distribute this software is granted
// provided this copyright notice appears in all copies. This software
// is provided "as is" without express or implied warranty, and with
// no claim as to its suitability for any purpose.
//
// Compilers Tested:
// =================
// Metrowerks Codewarrior Pro 7.2, 8.3
// gcc 2.95.3
// gcc 3.2
// Microsoft VC 6sp5 (test fails due to some compiler bug)
// Microsoft VC 7 (works)
// Microsoft VC 7.1
// Intel 5
// Intel 6
// Intel 7.1
// Intel 8
// Borland 5.5.1 (broken due to lack of support from Boost.Tuples)

#ifndef BOOST_ZIP_ITERATOR_TMB_07_13_2003_HPP_
# define BOOST_ZIP_ITERATOR_TMB_07_13_2003_HPP_

#include <stddef.h>
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_adaptor.hpp> // for enable_if_convertible
#include <boost/iterator/iterator_categories.hpp>
#include <boost/detail/iterator.hpp>

#include <boost/iterator/iterator_traits.hpp>
#include <boost/iterator/detail/minimum_category.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/spirit/fusion/algorithm/transform.hpp>
#include <boost/spirit/fusion/algorithm/for_each.hpp>
#include <boost/spirit/fusion/algorithm/fold.hpp>
#include <boost/spirit/fusion/sequence/generate.hpp>
#include <boost/spirit/fusion/sequence/begin.hpp>
#include <boost/spirit/fusion/algorithm/push_front.hpp>
#include <boost/spirit/fusion/sequence/equal_to.hpp>

namespace boost {

  // Zip iterator forward declaration for zip_iterator_base
  template<typename IteratorTuple>
  class zip_iterator;

  namespace detail
  {

    // Functors to be used with tuple algorithms
    //
    template<typename DiffType>
    class advance_iterator
    {
    public:
      advance_iterator(DiffType step) : m_step(step) {}
      
      template<typename Iterator>
      void operator()(Iterator& it) const
      { it += m_step; }

    private:
      DiffType m_step;
    };
    //
    struct increment_iterator
    {
      template<typename Iterator>
      void operator()(Iterator& it) const
      { ++it; }
    };
    //
    struct decrement_iterator
    {
      template<typename Iterator>
      void operator()(Iterator& it) const
      { --it; }
    };
    //
    struct dereference_iterator
    {
      template<typename Iterator>
      struct apply : iterator_reference<Iterator>
      {};

      template<typename Iterator>
      typename apply<Iterator>::type
      operator()(Iterator const& it) const
      { return *it; }
    };

    struct dereference
    {
        template <class Iterator>
        struct apply
          : boost::iterator_reference<Iterator>
        {};

        template <class Iterator>
        typename apply<Iterator>::type operator()(Iterator const& x) const
        {
            return *x;
        }
    };
    
    // Metafunction to obtain the type of the tuple whose element types
    // are the reference types of an iterator tuple.
    //
    template<typename IteratorTuple>
    struct tuple_of_references
      : mpl::transform<IteratorTuple, dereference::apply<mpl::_1> >
    {};
    
    // Metafunction to obtain the minimal traversal tag in a tuple
    // of iterators.
    //
    template<typename IteratorTuple>
    struct minimum_traversal_category_in_iterator_tuple
      : mpl::fold<
            typename mpl::transform<
                IteratorTuple, iterator_traversal<>
            >::type
          , random_access_traversal_tag
          , minimum_category<>
        >
    {};

#if BOOST_WORKAROUND(BOOST_MSVC, < 1300) // ETI workaround
      template <>
      struct minimum_traversal_category_in_iterator_tuple<int>
      {
          typedef int type;
      };
#endif
      
      // We need to call tuple_meta_accumulate with mpl::and_ as the
      // accumulating functor. To this end, we need to wrap it into
      // a struct that has exactly two arguments (that is, template
      // parameters) and not five, like mpl::and_ does.
      //
      template<typename Arg1, typename Arg2>
      struct and_with_two_args
        : mpl::and_<Arg1, Arg2>
      {
      };
    
# ifdef BOOST_MPL_CFG_NO_FULL_LAMBDA_SUPPORT
  // Hack because BOOST_MPL_AUX_LAMBDA_SUPPORT doesn't seem to work
  // out well.  In this case I think it's an MPL bug
      template<>
      struct and_with_two_args<mpl::_1,mpl::_2>
      {
          template <class A1, class A2>
          struct apply : mpl::and_<A1,A2>
          {};
      };
# endif 

    ///////////////////////////////////////////////////////////////////
    //
    // Class zip_iterator_base
    //
    // Builds and exposes the iterator facade type from which the zip 
    // iterator will be derived.
    //
    template<typename IteratorTuple>
    struct zip_iterator_base
    {
     private:
        // Reference type is the type of the tuple obtained from the
        // iterators' reference types.
        typedef typename 
        detail::tuple_of_references<IteratorTuple>::type reference;
      
        // Value type is the same as reference type.
        typedef reference value_type;
      
        // Difference type is the first iterator's difference type
        typedef typename iterator_traits<
            typename mpl::front<IteratorTuple>::type
            >::difference_type difference_type;
      
        // Traversal catetgory is the minimum traversal category in the 
        // iterator tuple.
        typedef typename 
        detail::minimum_traversal_category_in_iterator_tuple<
            IteratorTuple
        >::type traversal_category;
     public:
      
        // The iterator facade type from which the zip iterator will
        // be derived.
        typedef iterator_facade<
            zip_iterator<IteratorTuple>,
            value_type,  
            traversal_category,
            reference,
            difference_type
        > type;
    };

    template <>
    struct zip_iterator_base<int>
    {
        typedef int type;
    };
  }
  
  /////////////////////////////////////////////////////////////////////
  //
  // zip_iterator class definition
  //
  template<typename IteratorTuple>
  class zip_iterator : 
    public detail::zip_iterator_base<IteratorTuple>::type
  {  

   // Typedef super_t as our base class. 
   typedef typename 
     detail::zip_iterator_base<IteratorTuple>::type super_t;

   // iterator_core_access is the iterator's best friend.
   friend class iterator_core_access;

  public:
    
    // Construction
    // ============
    
    // Default constructor
    zip_iterator() { }

    // Constructor from iterator tuple
    zip_iterator(IteratorTuple iterator_tuple) 
      : m_iterator_tuple(iterator_tuple) 
    { }

    // Copy constructor
    template<typename OtherIteratorTuple>
    zip_iterator(
       const zip_iterator<OtherIteratorTuple>& other,
       typename enable_if_convertible<
         OtherIteratorTuple,
         IteratorTuple
         >::type* = 0
    ) : m_iterator_tuple(other.get_iterator_tuple())
    {}

    // Get method for the iterator tuple.
    const IteratorTuple& get_iterator_tuple() const
    { return m_iterator_tuple; }

  private:
    
    // Implementation of Iterator Operations
    // =====================================
    
    // Dereferencing returns a tuple built from the dereferenced
    // iterators in the iterator tuple.
    typename super_t::reference dereference() const
    { 
        return fusion::generate(fusion::transform( 
            get_iterator_tuple(),
            detail::dereference_iterator()
        ));
    }

    // Two zip iterators are equal if all iterators in the iterator
    // tuple are equal. NOTE: It should be possible to implement this
    // as
    //
    // return get_iterator_tuple() == other.get_iterator_tuple();
    //
    // but equality of tuples currently (7/2003) does not compile
    // under several compilers. No point in bringing in a bunch
    // of #ifdefs here.
    //
    template<typename OtherIteratorTuple>   
    bool equal(const zip_iterator<OtherIteratorTuple>& other) const
    {
        return get_iterator_tuple() == other.get_iterator_tuple();
    }

    // Advancing a zip iterator means to advance all iterators in the
    // iterator tuple.
    void advance(typename super_t::difference_type n)
    { 
        fusion::for_each(
            m_iterator_tuple,
            detail::advance_iterator<BOOST_DEDUCED_TYPENAME super_t::difference_type>(n)
          );
    }
    // Incrementing a zip iterator means to increment all iterators in
    // the iterator tuple.
    void increment()
    { 
        fusion::for_each(
            m_iterator_tuple,
            detail::increment_iterator()
        );
    }
    
    // Decrementing a zip iterator means to decrement all iterators in
    // the iterator tuple.
    void decrement()
    { 
        fusion::for_each(
            m_iterator_tuple,
            detail::decrement_iterator()
        );
    }
    
    // Distance is calculated using the first iterator in the tuple.
    template<typename OtherIteratorTuple>
      typename super_t::difference_type distance_to(
        const zip_iterator<OtherIteratorTuple>& other
        ) const
    { 
        return *fusion::begin(other.get_iterator_tuple()) - 
            *fusion::begin(this->get_iterator_tuple());
    }
  
    // Data Members
    // ============
    
    // The iterator tuple.
    IteratorTuple m_iterator_tuple;
 
  };

  // Make function for zip iterator
  //
  template<typename IteratorTuple> 
  zip_iterator<IteratorTuple> 
  make_zip_iterator(IteratorTuple t)
  { return zip_iterator<IteratorTuple>(t); }

}

#endif
