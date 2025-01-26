// Copyright David Abrahams 2003. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef IS_LVALUE_ITERATOR_DWA2003112_HPP
# define IS_LVALUE_ITERATOR_DWA2003112_HPP

#include <boost/detail/workaround.hpp>

#include <boost/iterator/detail/type_traits/conjunction.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>

#include <iterator>
#include <type_traits>

namespace boost {

namespace iterators {

namespace detail
{
  // Guts of is_lvalue_iterator.  Value is the iterator's value_type
  // and the result is computed in the nested rebind template.
  template <class Value>
  struct is_lvalue_iterator_impl
  {
      template <class It>
      using DerefT = decltype(*std::declval<It&>());

      template <class It>
      struct rebind : detail::conjunction<
                        std::is_convertible<DerefT<It>, typename std::add_lvalue_reference<Value>::type>
                      , std::is_lvalue_reference<DerefT<It>>
                      >::type
      {
      };
  };

  //
  // void specializations to handle std input and output iterators
  //
  template <>
  struct is_lvalue_iterator_impl<void>
  {
      template <class It>
      struct rebind : std::false_type
      {};
  };

#ifndef BOOST_NO_CV_VOID_SPECIALIZATIONS
  template <>
  struct is_lvalue_iterator_impl<const void>
  {
      template <class It>
      struct rebind : std::false_type
      {};
  };

  template <>
  struct is_lvalue_iterator_impl<volatile void>
  {
      template <class It>
      struct rebind : std::false_type
      {};
  };

  template <>
  struct is_lvalue_iterator_impl<const volatile void>
  {
      template <class It>
      struct rebind : std::false_type
      {};
  };
#endif

  //
  // This level of dispatching is required for Borland.  We might save
  // an instantiation by removing it for others.
  //
  template <class It>
  struct is_readable_lvalue_iterator_impl
    : is_lvalue_iterator_impl<
          BOOST_DEDUCED_TYPENAME std::iterator_traits<It>::value_type const
      >::template rebind<It>
  {};

  template <class It>
  struct is_non_const_lvalue_iterator_impl
    : is_lvalue_iterator_impl<
          BOOST_DEDUCED_TYPENAME std::iterator_traits<It>::value_type
      >::template rebind<It>
  {};
} // namespace detail

template< typename T > struct is_lvalue_iterator
: public std::integral_constant<bool,::boost::iterators::detail::is_readable_lvalue_iterator_impl<T>::value>
{
public:
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1,is_lvalue_iterator,(T))
};

template< typename T > struct is_non_const_lvalue_iterator
: public std::integral_constant<bool,::boost::iterators::detail::is_non_const_lvalue_iterator_impl<T>::value>
{
public:
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1,is_non_const_lvalue_iterator,(T))
};

} // namespace iterators

using iterators::is_lvalue_iterator;
using iterators::is_non_const_lvalue_iterator;

} // namespace boost

#endif // IS_LVALUE_ITERATOR_DWA2003112_HPP
