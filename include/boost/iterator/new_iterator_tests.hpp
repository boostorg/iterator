#ifndef BOOST_NEW_ITERATOR_TESTS_HPP
# define BOOST_NEW_ITERATOR_TESTS_HPP

// This is meant to be the beginnings of a comprehensive, generic
// test suite for STL concepts such as iterators and containers.
//
// Revision History:
// 28 Oct 2002  Started update for new iterator categories
//              (Jeremy Siek)
// 28 Apr 2002  Fixed input iterator requirements.
//              For a == b a++ == b++ is no longer required.
//              See 24.1.1/3 for details.
//              (Thomas Witt)
// 08 Feb 2001  Fixed bidirectional iterator test so that
//              --i is no longer a precondition.
//              (Jeremy Siek)
// 04 Feb 2001  Added lvalue test, corrected preconditions
//              (David Abrahams)

# include <iterator>
# include <assert.h>
# include <boost/type_traits.hpp>
# include <boost/static_assert.hpp>
# include <boost/concept_archetype.hpp> // for detail::dummy_constructor
#include <boost/detail/iterator.hpp>
#include <boost/pending/iterator_tests.hpp>

namespace boost {

void is_readable(readable_iterator_tag) { }
void is_writable(writable_iterator_tag) { }
void is_swappable(swappable_iterator_tag) { }
void is_constant_lvalue(constant_lvalue_iterator_tag) { }
void is_mutable_lvalue(mutable_lvalue_iterator_tag) { }

// Preconditions: *i == v
template <class Iterator, class T>
void readable_iterator_test(const Iterator i1, T v)
{
  Iterator i2(i1); // Copy Constructible
  typedef typename detail::iterator_traits<Iterator>::reference ref_t;
  ref_t r1 = *i1;
  ref_t r2 = *i2;
  T v1 = r1;
  T v2 = r2;
  assert(v1 == v);
  assert(v2 == v);
  typedef typename return_category<Iterator>::type result_category;
  is_readable(result_category());
}

template <class Iterator, class T>
void writable_iterator_test(Iterator i, T v)
{
  Iterator i2(i); // Copy Constructible
  *i2 = v;
  is_writable(typename return_category<Iterator>::type());
}

template <class Iterator>
void swappable_iterator_test(Iterator i, Iterator j)
{
  Iterator i2(i), j2(j);
  typename detail::iterator_traits<Iterator>::value_type bi = *i, bj = *j;
  iter_swap(i2, j2);
  typename detail::iterator_traits<Iterator>::value_type ai = *i, aj = *j;
  assert(bi == aj && bj == ai);
  typedef typename return_category<Iterator>::type result_category;
  is_swappable(result_category());
}

template <class Iterator, class T>
void constant_lvalue_iterator_test(Iterator i, T v1)
{
  Iterator i2(i);
  typedef typename detail::iterator_traits<Iterator>::value_type value_type;
  typedef typename detail::iterator_traits<Iterator>::reference reference;
  BOOST_STATIC_ASSERT((is_same<const value_type&, reference>::value));
  const T& v2 = *i2;
  assert(v1 == v2);
  typedef typename return_category<Iterator>::type result_category;
  is_constant_lvalue(result_category());
}

template <class Iterator, class T>
void mutable_lvalue_iterator_test(Iterator i, T v1, T v2)
{
  Iterator i2(i);
  typedef typename detail::iterator_traits<Iterator>::value_type value_type;
  typedef typename detail::iterator_traits<Iterator>::reference reference;
  BOOST_STATIC_ASSERT((is_same<value_type&, reference>::value));
  T& v3 = *i2;
  assert(v1 == v3);
  *i = v2;
  T& v4 = *i2;
  assert(v2 == v4);
  typedef typename return_category<Iterator>::type result_category;
  is_mutable_lvalue(result_category());
}

template <class Iterator, class T>
void forward_readable_iterator_test(Iterator i, Iterator j, T val1, T val2)
{
  Iterator i2;
  Iterator i3(i);
  i2 = i;
  assert(i2 == i3);
  assert(i != j);
  assert(i2 != j);
  readable_iterator_test(i, val1);
  readable_iterator_test(i2, val1);
  readable_iterator_test(i3, val1);

  assert(i == i2++);
  assert(i != ++i3);

  readable_iterator_test(i2, val2);
  readable_iterator_test(i3, val2);

  readable_iterator_test(i, val1);
}

template <class Iterator, class T>
void forward_swappable_iterator_test(Iterator i, Iterator j, T val1, T val2)
{
  forward_readable_iterator_test(i, j, val1, val2);
  Iterator i2 = i;
  ++i2;
  swappable_iterator_test(i, i2);
}

// bidirectional
// Preconditions: *i == v1, *++i == v2
template <class Iterator, class T>
void bidirectional_readable_iterator_test(Iterator i, T v1, T v2)
{
  Iterator j(i);
  ++j;
  forward_readable_iterator_test(i, j, v1, v2);
  ++i;

  Iterator i1 = i, i2 = i;

  assert(i == i1--);
  assert(i != --i2);

  readable_iterator_test(i, v2);
  readable_iterator_test(i1, v1);
  readable_iterator_test(i2, v1);

  --i;
  assert(i == i1);
  assert(i == i2);
  ++i1;
  ++i2;

  readable_iterator_test(i, v1);
  readable_iterator_test(i1, v2);
  readable_iterator_test(i2, v2);
}


// random access
// Preconditions: [i,i+N) is a valid range
template <class Iterator, class TrueVals>
void random_access_readable_iterator_test(Iterator i, int N, TrueVals vals)
{
  bidirectional_readable_iterator_test(i, vals[0], vals[1]);
  const Iterator j = i;
  int c;

  for (c = 0; c < N-1; ++c) {
    assert(i == j + c);
    assert(*i == vals[c]);
    assert(*i == j[c]);
    assert(*i == *(j + c));
    assert(*i == *(c + j));
    ++i;
    assert(i > j);
    assert(i >= j);
    assert(j <= i);
    assert(j < i);
  }

  Iterator k = j + N - 1;
  for (c = 0; c < N-1; ++c) {
    assert(i == k - c);
    assert(*i == vals[N - 1 - c]);
    assert(*i == j[N - 1 - c]);
    Iterator q = k - c; 
    assert(*i == *q);
    assert(i > j);
    assert(i >= j);
    assert(j <= i);
    assert(j < i);
    --i;
  }
}


#if 0

// Tests whether type Iterator satisfies the requirements for a
// TrivialIterator. 
// Preconditions: i != j, *i == val
template <class Iterator, class T>
void trivial_iterator_test(const Iterator i, const Iterator j, T val)
{
  Iterator k;
  assert(i == i);
  assert(j == j);
  assert(i != j);
#ifdef BOOST_NO_STD_ITERATOR_TRAITS
  T v = *i;
#else
  typename std::iterator_traits<Iterator>::value_type v = *i;
#endif
  assert(v == val);
#if 0
  // hmm, this will give a warning for transform_iterator...  perhaps
  // this should be separated out into a stand-alone test since there
  // are several situations where it can't be used, like for
  // integer_range::iterator.
  assert(v == i->foo());
#endif
  k = i;
  assert(k == k);
  assert(k == i);
  assert(k != j);
  assert(*k == val);
}


// Preconditions: i != j
template <class Iterator, class T>
void mutable_trivial_iterator_test(const Iterator i, const Iterator j, T val)
{
  *i = val;
  trivial_iterator_test(i, j, val);
}





template <class Iterator, class T>
void dereference_test(Iterator i, T v1, read_write_iterator_tag)
{
  *i = v1;
}


// Preconditions: *i == v1, *++i == v2
template <class Iterator, class T>
void input_iterator_test(Iterator i, T v1, T v2) 
{
  Iterator i1(i);

  assert(i == i1);
  assert(!(i != i1));

  // I can see no generic way to create an input iterator
  // that is in the domain of== of i and != i.
  // The following works for istream_iterator but is not
  // guaranteed to work for arbitrary input iterators.
  //
  //   Iterator i2;
  //
  //   assert(i != i2);
  //   assert(!(i == i2));

  assert(*i1 == v1);
  assert(*i  == v1);

  // we cannot test for equivalence of (void)++i & (void)i++
  // as i is only guaranteed to be single pass.
  assert(*i++ == v1);

  i1 = i;

  assert(i == i1);
  assert(!(i != i1));

  assert(*i1 == v2);
  assert(*i  == v2);

  // i is dereferencable, so it must be incrementable.
  ++i;

  // how to test for operator-> ?
}

// how to test output iterator?


template <bool is_pointer> struct lvalue_test
{
    template <class Iterator> static void check(Iterator)
    {
# ifndef BOOST_NO_STD_ITERATOR_TRAITS
        typedef typename std::iterator_traits<Iterator>::reference reference;
        typedef typename std::iterator_traits<Iterator>::value_type value_type;
# else
        typedef typename Iterator::reference reference;
        typedef typename Iterator::value_type value_type;
# endif
        BOOST_STATIC_ASSERT(boost::is_reference<reference>::value);
        BOOST_STATIC_ASSERT((boost::is_same<reference,value_type&>::value
                             || boost::is_same<reference,const value_type&>::value
            ));
    }
};

# ifdef BOOST_NO_STD_ITERATOR_TRAITS
template <> struct lvalue_test<true> {
    template <class T> static void check(T) {}
};
#endif

template <class Iterator, class T>
void forward_iterator_test(Iterator i, T v1, T v2) 
{
  input_iterator_test(i, v1, v2);

  Iterator i1 = i, i2 = i;

  assert(i == i1++);
  assert(i != ++i2);

  trivial_iterator_test(i, i1, v1);
  trivial_iterator_test(i, i2, v1);

  ++i;
  assert(i == i1);
  assert(i == i2);
  ++i1;
  ++i2;

  trivial_iterator_test(i, i1, v2);
  trivial_iterator_test(i, i2, v2);

 // borland doesn't allow non-type template parameters
# if !defined(__BORLANDC__) || (__BORLANDC__ > 0x551)
  lvalue_test<(boost::is_pointer<Iterator>::value)>::check(i);
#endif
}

// Preconditions: *i == v1, *++i == v2
template <class Iterator, class T>
void bidirectional_iterator_test(Iterator i, T v1, T v2)
{
  forward_iterator_test(i, v1, v2);
  ++i;

  Iterator i1 = i, i2 = i;

  assert(i == i1--);
  assert(i != --i2);

  trivial_iterator_test(i, i1, v2);
  trivial_iterator_test(i, i2, v2);

  --i;
  assert(i == i1);
  assert(i == i2);
  ++i1;
  ++i2;

  trivial_iterator_test(i, i1, v1);
  trivial_iterator_test(i, i2, v1);
}

// mutable_bidirectional_iterator_test

// Preconditions: [i,i+N) is a valid range
template <class Iterator, class TrueVals>
void random_access_iterator_test(Iterator i, int N, TrueVals vals)
{
  bidirectional_iterator_test(i, vals[0], vals[1]);
  const Iterator j = i;
  int c;

  for (c = 0; c < N-1; ++c) {
    assert(i == j + c);
    assert(*i == vals[c]);
    assert(*i == j[c]);
    assert(*i == *(j + c));
    assert(*i == *(c + j));
    ++i;
    assert(i > j);
    assert(i >= j);
    assert(j <= i);
    assert(j < i);
  }

  Iterator k = j + N - 1;
  for (c = 0; c < N-1; ++c) {
    assert(i == k - c);
    assert(*i == vals[N - 1 - c]);
    assert(*i == j[N - 1 - c]);
    Iterator q = k - c; 
    assert(*i == *q);
    assert(i > j);
    assert(i >= j);
    assert(j <= i);
    assert(j < i);
    --i;
  }
}

// Precondition: i != j
template <class Iterator, class ConstIterator>
void const_nonconst_iterator_test(Iterator i, ConstIterator j)
{
  assert(i != j);
  assert(j != i);

  ConstIterator k(i);
  assert(k == i);
  assert(i == k);

  k = i;
  assert(k == i);
  assert(i == k);
}

#endif

} // namespace boost

#endif // BOOST_NEW_ITERATOR_TESTS_HPP
