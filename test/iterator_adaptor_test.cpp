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
        ptr_iterator<V>
      , V*
      , V
      , std::random_access_iterator_tag
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
      , V&, V*
#endif 
   >
{
private:
  typedef boost::iterator_adaptor<
        ptr_iterator<V>
      , V*
      , V
      , std::random_access_iterator_tag
#if BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
      , V&, V*
#endif 
    > super_t;
    
public:
  ptr_iterator() { }
  ptr_iterator(V* d) : super_t(d) { }

  template <class V2>
  ptr_iterator(
        const ptr_iterator<V2>& x
      , typename boost::enable_if_convertible<V2, V>::type* = 0
  )
    : super_t(x.base())
  {}
};

template <class T>
struct fwd_iterator
  : boost::iterator_adaptor<
        fwd_iterator<T>
      , boost::forward_iterator_archetype<T>
    >
{
private:
  typedef boost::iterator_adaptor<
        fwd_iterator<T>
      , boost::forward_iterator_archetype<T>
  > super_t;
    
public:
  fwd_iterator() { }
  fwd_iterator(boost::forward_iterator_archetype<T> d) : super_t(d) { }
};

template <class T>
struct in_iterator
  : boost::iterator_adaptor<
        in_iterator<T>
      , boost::input_iterator_archetype<T>
    >
{
private:
  typedef boost::iterator_adaptor<
        in_iterator<T>
      , boost::input_iterator_archetype<T>
  > super_t;
    
public:
  in_iterator() { }
  in_iterator(boost::input_iterator_archetype<T> d) : super_t(d) { }
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
    typedef ptr_iterator<int> Iter1;
    // don't use std::iterator_traits here to avoid VC++ problems
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::value_type, int>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, int&>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, int*>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::difference_type, std::ptrdiff_t>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::iterator_category, std::random_access_iterator_tag>::value));
  }
  {  
    // Test computation of default when the Value is const
    typedef ptr_iterator<int const> Iter1;
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::value_type, int>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::reference, const int&>::value));
    BOOST_STATIC_ASSERT((boost::is_same<Iter1::pointer, const int*>::value));
  }
  
  // Test the iterator_adaptor
  {
    ptr_iterator<dummyT> i(array);
    boost::random_access_iterator_test(i, N, array);
    
    ptr_iterator<const dummyT> j(array);
    boost::random_access_iterator_test(j, N, array);
    boost::const_nonconst_iterator_test(i, ++j);
  }

  // check operator-> with a forward iterator
  {
    boost::forward_iterator_archetype<dummyT> forward_iter;

    typedef fwd_iterator<dummyT> adaptor_type;

    adaptor_type i(forward_iter);
    int zero = 0;
    if (zero) // don't do this, just make sure it compiles
      assert((*i).m_x == i->foo());      
  }
  
  // check operator-> with an input iterator
  {
    boost::input_iterator_archetype<dummyT> input_iter;
    typedef in_iterator<dummyT> adaptor_type;
    adaptor_type i(input_iter);
    int zero = 0;
    if (zero) // don't do this, just make sure it compiles
      assert((*i).m_x == i->foo());      
  }

  std::cout << "test successful " << std::endl;
  return 0;
}
