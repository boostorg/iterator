//  (C) Copyright Thomas Witt 2003. Permission to copy, use, modify,
//  sell and distribute this software is granted provided this
//  copyright notice appears in all copies. This software is provided
//  "as is" without express or implied warranty, and with no claim as
//  to its suitability for any purpose.

//  See http://www.boost.org for most recent version including documentation.

#include <boost/config.hpp>
#include <iostream>

#include <algorithm>
#include <functional>
#include <numeric>

#if 0
#include <boost/iterator_adaptors.hpp>
#include <boost/generator_iterator.hpp>
#include <boost/pending/integer_range.hpp>
#include <boost/concept_archetype.hpp>
#include <boost/type_traits/same_traits.hpp>
#include <boost/permutation_iterator.hpp>
#endif

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/pending/iterator_tests.hpp>

#include <stdlib.h>
#include <vector>
#include <deque>
#include <set>
#include <list>

struct my_iterator_tag : public std::random_access_iterator_tag { };

using boost::dummyT;


struct mult_functor {
  typedef int result_type;
  typedef int argument_type;
  // Functors used with transform_iterator must be
  // DefaultConstructible, as the transform_iterator must be
  // DefaultConstructible to satisfy the requirements for
  // TrivialIterator.
  mult_functor() { }
  mult_functor(int aa) : a(aa) { }
  int operator()(int b) const { return a * b; }
  int a;
};

template <class Pair>
struct select1st_ 
  : public std::unary_function<Pair, typename Pair::first_type>
{
  const typename Pair::first_type& operator()(const Pair& x) const {
    return x.first;
  }
  typename Pair::first_type& operator()(Pair& x) const {
    return x.first;
  }
};

struct one_or_four {
  bool operator()(dummyT x) const {
    return x.foo() == 1 || x.foo() == 4;
  }
};

typedef std::deque<int> storage;
typedef std::deque<int*> pointer_deque;
typedef std::set<storage::iterator> iterator_set;

template <class T> struct foo;

void blah(int) { }

struct my_gen
{
  typedef int result_type;
  my_gen() : n(0) { }
  int operator()() { return ++n; }
  int n;
};

template <class V>
struct ptr_iterator
  : boost::iterator_adaptor<
      ptr_iterator<V>,
      boost::iterator_traits_adaptor<V*,
                                     V, 
                                     boost::iterator_tag< boost::writable_lvalue_iterator_tag
                                                          ,boost::random_access_traversal_tag > >
    >
{
private:
  typedef iterator_adaptor super_t;
public:
  ptr_iterator() { }
  ptr_iterator(V* d) : super_t(d) { }

  template <class V2>
  ptr_iterator(const ptr_iterator<V2>& x)  // should assert is_same add_cv<V> add_cv<V2>
    : super_t(x.base()) { }

};

int
main()
{
  dummyT array[] = { dummyT(0), dummyT(1), dummyT(2), 
                     dummyT(3), dummyT(4), dummyT(5) };
  const int N = sizeof(array)/sizeof(dummyT);

  // sanity check, if this doesn't pass the test is buggy
  boost::random_access_iterator_test(array, N, array);

  // Test the iterator_adaptor
  {
    ptr_iterator<dummyT> i(array);
    boost::random_access_iterator_test(i, N, array);
    
    ptr_iterator<const dummyT> j(array);
    boost::random_access_iterator_test(j, N, array);
    boost::const_nonconst_iterator_test(i, ++j);
  }
  
  // Test the iterator_traits
  {
    // Test computation of defaults
    typedef boost::iterator_adaptor<int*, boost::default_iterator_policies,
      boost::value_type_is<int> > Iter1;
    // don't use std::iterator_traits here to avoid VC++ problems
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::value_type, int>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, int&>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, int*>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::difference_type, std::ptrdiff_t>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::iterator_category, std::random_access_iterator_tag>::value));
  }
  {  
    // Test computation of default when the Value is const
    typedef boost::iterator_adaptor<std::list<int>::iterator,
      boost::default_iterator_policies,
      boost::value_type_is<const int> > Iter1;
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::value_type, int>::value));
#if defined(__BORLANDC__) || defined(BOOST_MSVC) && BOOST_MSVC <= 1300
    // We currently don't know how to workaround this bug.
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, int&>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, int*>::value));
#else
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, const int&>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, const int*>::value));
#endif
  }
  {
    // Test with no defaults
    typedef boost::iterator_adaptor<int*, boost::default_iterator_policies,
      boost::reference_is<long>,
      boost::pointer_is<float*>,
      boost::value_type_is<char>,
      boost::iterator_category_is<std::input_iterator_tag>,
      boost::difference_type_is<int>
    > Iter1;
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::value_type, char>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, long>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, float*>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::difference_type, int>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::iterator_category, std::input_iterator_tag>::value));
  }
  // Test the iterator_adaptor
  {
    boost::iterator_adaptor<dummyT*, boost::default_iterator_policies, dummyT> i(array);
    boost::random_access_iterator_test(i, N, array);
    
    boost::iterator_adaptor<const dummyT*, boost::default_iterator_policies, const dummyT> j(array);
    boost::random_access_iterator_test(j, N, array);
    boost::const_nonconst_iterator_test(i, ++j);
  }

  // check operator-> with a forward iterator
  {
    boost::forward_iterator_archetype<dummyT> forward_iter;
#if defined(__BORLANDC__)
    typedef boost::iterator_adaptor<boost::forward_iterator_archetype<dummyT>,
      boost::default_iterator_policies,
      dummyT, const dummyT&, const dummyT*, 
      std::forward_iterator_tag, std::ptrdiff_t> adaptor_type;
#else
    typedef boost::iterator_adaptor<boost::forward_iterator_archetype<dummyT>,
      boost::default_iterator_policies,
      boost::reference_is<const dummyT&>,
      boost::pointer_is<const dummyT*> ,
      boost::iterator_category_is<std::forward_iterator_tag>,
      boost::value_type_is<dummyT>,
      boost::difference_type_is<std::ptrdiff_t>
    > adaptor_type;
#endif
    adaptor_type i(forward_iter);
    int zero = 0;
    if (zero) // don't do this, just make sure it compiles
      assert((*i).m_x == i->foo());      
  }
  // check operator-> with an input iterator
  {
    boost::input_iterator_archetype<dummyT> input_iter;
    typedef boost::iterator_adaptor<boost::input_iterator_archetype<dummyT>,
      boost::default_iterator_policies,
      dummyT, const dummyT&, const dummyT*, 
      std::input_iterator_tag, std::ptrdiff_t> adaptor_type;
    adaptor_type i(input_iter);
    int zero = 0;
    if (zero) // don't do this, just make sure it compiles
      assert((*i).m_x == i->foo());      
  }

  std::cout << "test successful " << std::endl;
  return 0;
}
