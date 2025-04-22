//  (C) Copyright Jeremy Siek 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Revision History
//  22 Nov 2002 Thomas Witt
//       Added interoperability check.
//  28 Oct 2002   Jeremy Siek
//       Updated for new iterator adaptors.
//  08 Mar 2001   Jeremy Siek
//       Moved test of transform iterator into its own file. It to
//       to be in iterator_adaptor_test.cpp.

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/iterator/new_iterator_tests.hpp>
#include <boost/pending/iterator_tests.hpp>
#include <boost/concept_check.hpp>

#if defined(__cpp_lib_ranges) && ( __cpp_lib_ranges >= 202002L )
#include <ranges>
#endif

#include "static_assert_same.hpp"

struct mult_functor {
  // Functors used with transform_iterator must be
  // DefaultConstructible, as the transform_iterator must be
  // DefaultConstructible to satisfy the requirements for
  // TrivialIterator.
  mult_functor() { }
  mult_functor(int aa) : a(aa) { }
  int operator()(int b) const { return a * b; }
  int a;
};

struct adaptable_mult_functor
 : mult_functor
{
  typedef int result_type;
  typedef int argument_type;
  // Functors used with transform_iterator must be
  // DefaultConstructible, as the transform_iterator must be
  // DefaultConstructible to satisfy the requirements for
  // TrivialIterator.
  adaptable_mult_functor() { }
  adaptable_mult_functor(int aa) : mult_functor(aa) { }
};

struct identity {
  template <class T>
  T&& operator()(T&& x) const {
    return static_cast<T&&>(x);
  }
};

struct reference_to_value {
  template <class T>
  typename std::remove_reference<T>::type operator()(T&& x) const {
    return x;
  }
};

struct const_select_first
{
  typedef int const& result_type;

  int const& operator()(std::pair<int, int>const& p) const
  {
    return p.first;
  }
};

struct select_first
  : const_select_first // derivation to allow conversions
{
  typedef int& result_type;

  int& operator()(std::pair<int, int>& p) const
  {
    return p.first;
  }
};

struct select_second
{
  typedef int& result_type;

  int& operator()(std::pair<int, int>& p) const
  {
    return p.second;
  }
};

struct value_select_first
{
  typedef int result_type;

  int operator()(std::pair<int, int>const& p) const
  {
    return p.first;
  }
};

int mult_2(int arg)
{
  return arg*2;
}

struct polymorphic_mult_functor
{
    //Implement result_of protocol
    template <class FArgs> struct result;
    template <class F, class T> struct result<const F(T       )> {typedef T type;};
    template <class F, class T> struct result<const F(T&      )> {typedef T type;};
    template <class F, class T> struct result<const F(const T&)> {typedef T type;};
    template <class F, class T> struct result<F(T       )> {typedef void type;};
    template <class F, class T> struct result<F(T&      )> {typedef void type;};
    template <class F, class T> struct result<F(const T&)> {typedef void type;};

    template <class T>
    T operator()(const T& _arg) const {return _arg*2;}
    template <class T>
    void operator()(const T& _arg) { BOOST_ASSERT(0); }
};

int
main()
{
  const int N = 10;

  // Concept checks
  {
    typedef boost::transform_iterator<adaptable_mult_functor, int*>       iter_t;
    typedef boost::transform_iterator<adaptable_mult_functor, int const*> c_iter_t;

    boost::function_requires< boost_concepts::InteroperableIteratorConcept<iter_t, c_iter_t> >();
  }

  // Test transform_iterator
  {
    int x[N], y[N];
    for (int k = 0; k < N; ++k)
      x[k] = k;
    std::copy(x, x + N, y);

    for (int k2 = 0; k2 < N; ++k2)
      x[k2] = x[k2] * 2;

    typedef boost::transform_iterator<adaptable_mult_functor, int*> iter_t;
    iter_t i(y, adaptable_mult_functor(2));
    boost::input_iterator_test(i, x[0], x[1]);
    boost::input_iterator_test(iter_t(&y[0], adaptable_mult_functor(2)), x[0], x[1]);

    boost::random_access_readable_iterator_test(i, N, x);
  }

  // Test transform_iterator non adaptable functor
  {
    int x[N], y[N];
    for (int k = 0; k < N; ++k)
      x[k] = k;
    std::copy(x, x + N, y);

    for (int k2 = 0; k2 < N; ++k2)
      x[k2] = x[k2] * 2;

    typedef boost::transform_iterator<mult_functor, int*, int> iter_t;
    iter_t i(y, mult_functor(2));
    boost::input_iterator_test(i, x[0], x[1]);
    boost::input_iterator_test(iter_t(&y[0], mult_functor(2)), x[0], x[1]);

    boost::random_access_readable_iterator_test(i, N, x);
  }

  // Test transform_iterator default argument handling
  {
    {
      typedef boost::transform_iterator<adaptable_mult_functor, int*, float> iter_t;
      STATIC_ASSERT_SAME(iter_t::reference,  float);
      STATIC_ASSERT_SAME(iter_t::value_type, float);
    }

    {
      typedef boost::transform_iterator<adaptable_mult_functor, int*, boost::use_default, float> iter_t;
      STATIC_ASSERT_SAME(iter_t::reference,  int);
      STATIC_ASSERT_SAME(iter_t::value_type, float);
    }

    {
      typedef boost::transform_iterator<adaptable_mult_functor, int*, float, double> iter_t;
      STATIC_ASSERT_SAME(iter_t::reference,  float);
      STATIC_ASSERT_SAME(iter_t::value_type, double);
    }
  }

  // Test transform_iterator with function pointers
  {
    int x[N], y[N];
    for (int k = 0; k < N; ++k)
      x[k] = k;
    std::copy(x, x + N, y);

    for (int k2 = 0; k2 < N; ++k2)
      x[k2] = x[k2] * 2;

    boost::input_iterator_test(
        boost::make_transform_iterator(y, mult_2), x[0], x[1]);

    boost::input_iterator_test(
        boost::make_transform_iterator(&y[0], mult_2), x[0], x[1]);

    boost::random_access_readable_iterator_test(
        boost::make_transform_iterator(y, mult_2), N, x);

  }

  // Test transform_iterator as projection iterator
  {
    typedef std::pair<int, int> pair_t;

    int    x[N];
    int    y[N];
    pair_t values[N];

    for(int i = 0; i < N; ++i) {

      x[i]             = i;
      y[i]             = N - (i + 1);

    }

    std::copy(
        x
      , x + N
      , boost::make_transform_iterator((pair_t*)values, select_first())
    );

    std::copy(
        y
      , y + N
      , boost::make_transform_iterator((pair_t*)values, select_second())
    );

    boost::random_access_readable_iterator_test(
        boost::make_transform_iterator((pair_t*)values, value_select_first())
      , N
      , x
    );

    boost::random_access_readable_iterator_test(
        boost::make_transform_iterator((pair_t*)values, const_select_first())
      , N, x
    );

    boost::constant_lvalue_iterator_test(
        boost::make_transform_iterator((pair_t*)values, const_select_first()), x[0]);

    boost::non_const_lvalue_iterator_test(
        boost::make_transform_iterator((pair_t*)values, select_first()), x[0], 17);

    boost::const_nonconst_iterator_test(
        ++boost::make_transform_iterator((pair_t*)values, select_first())
      , boost::make_transform_iterator((pair_t*)values, const_select_first())
    );
  }

  // Test transform_iterator with polymorphic object function
  {
    int x[N], y[N];
    for (int k = 0; k < N; ++k)
      x[k] = k;
    std::copy(x, x + N, y);

    for (int k2 = 0; k2 < N; ++k2)
      x[k2] = x[k2] * 2;

    boost::input_iterator_test(
        boost::make_transform_iterator(y, polymorphic_mult_functor()), x[0], x[1]);

    boost::input_iterator_test(
        boost::make_transform_iterator(&y[0], polymorphic_mult_functor()), x[0], x[1]);

    boost::random_access_readable_iterator_test(
        boost::make_transform_iterator(y, polymorphic_mult_functor()), N, x);
  }


  {
    using Iter = boost::iterators::transform_iterator<identity, typename std::vector<int>::iterator>;
    using ConstIter = boost::iterators::transform_iterator<identity, typename std::vector<int>::const_iterator>;

    static_assert(
      std::is_same<decltype(*std::declval<Iter>()), int&>::value,
      "Transform iterator with identity must dereference into int reference."
    );

    static_assert(
      std::is_same<decltype(*std::declval<ConstIter>()), const int&>::value,
      "Transform iterator with identity must dereference into const int reference."
    );

    static_assert(
      std::is_same<decltype(std::declval<Iter>()[std::declval<std::size_t>()]), int&>::value,
      "Transform iterator over iterator with identity must return int reference for operator[] call."
    );

    static_assert(
      std::is_same<decltype(std::declval<ConstIter>()[std::declval<std::size_t>()]), const int&>::value,
      "Transform iterator over const iterator with identity must return int reference for operator[] call."
    );

    #if defined(__cpp_lib_concepts) && ( __cpp_lib_concepts >= 202002L )
      static_assert(std::random_access_iterator<Iter>);
      static_assert(std::random_access_iterator<ConstIter>);
      static_assert(std::output_iterator<Iter, int>);
      static_assert(std::input_iterator<Iter>);
      static_assert(!std::output_iterator<ConstIter, int>);
      static_assert(std::input_iterator<ConstIter>);
    #endif

    auto nums = std::vector<int>{1, 2, 3, 4, 5, 6};

    auto iter1 = boost::iterators::make_transform_iterator<identity>(nums.begin());
    auto iter2 = boost::iterators::make_transform_iterator<identity>(nums.end());

    const auto found3 = std::lower_bound(iter1, iter2, 3);
    BOOST_TEST(*found3 == 3);

    #if defined(__cpp_lib_ranges)
    auto found3rng = std::ranges::lower_bound(iter1, iter2, 3);
    BOOST_TEST(*found3rng == 3);
    #endif

    *std::prev(iter2) = 7;
    BOOST_TEST(nums.back() == 7);
  }

  {
    using Iter = boost::iterators::transform_iterator<reference_to_value, typename std::vector<int>::iterator>;
    using ConstIter = boost::iterators::transform_iterator<reference_to_value, typename std::vector<int>::const_iterator>;

    static_assert(
      std::is_same<decltype(*std::declval<Iter>()), int>::value,
      "Transform iterator with identity must dereference into int reference."
    );

    static_assert(
      std::is_same<decltype(*std::declval<ConstIter>()), int>::value,
      "Transform iterator with identity must dereference into const int reference."
    );

    static_assert(
      std::is_same<decltype(std::declval<Iter>()[std::declval<std::size_t>()]), int>::value,
      "Transform iterator over iterator with identity must return int reference for operator[] call."
    );

    static_assert(
      std::is_same<decltype(std::declval<ConstIter>()[std::declval<std::size_t>()]), int>::value,
      "Transform iterator over const iterator with identity must return int reference for operator[] call."
    );

    #if defined(__cpp_lib_concepts) && ( __cpp_lib_concepts >= 202002L )
      static_assert(std::random_access_iterator<Iter>);
      static_assert(std::random_access_iterator<ConstIter>);
      static_assert(!std::output_iterator<Iter, int>);
      static_assert(std::input_iterator<Iter>);
      static_assert(!std::output_iterator<ConstIter, int>);
      static_assert(std::input_iterator<ConstIter>);
    #endif

    auto nums = std::vector<int>{1, 2, 3, 4, 5, 6};

    auto iter1 = boost::iterators::make_transform_iterator<identity>(nums.begin());
    auto iter2 = boost::iterators::make_transform_iterator<identity>(nums.end());

    const auto found3 = std::lower_bound(iter1, iter2, 3);
    BOOST_TEST(*found3 == 3);

    #if defined(__cpp_lib_ranges) && ( __cpp_lib_ranges >= 202002L )
    auto found3rng = std::ranges::lower_bound(iter1, iter2, 3);
    BOOST_TEST(*found3rng == 3);
    #endif
  }

  return boost::report_errors();
}
