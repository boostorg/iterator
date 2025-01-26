// Copyright David Abrahams 2003. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ITERATOR_MINIMUM_CATEGORY_HPP_INCLUDED_
#define BOOST_ITERATOR_MINIMUM_CATEGORY_HPP_INCLUDED_

#include <type_traits>
#include <boost/mpl/arg_fwd.hpp>

namespace boost {
namespace iterators {
namespace detail {

template <class T1, class T2, bool GreaterEqual, bool LessEqual>
struct minimum_category_impl;

template <class T1, class T2>
struct minimum_category_impl<T1, T2, true, false>
{
    using type = T2;
};

template <class T1, class T2>
struct minimum_category_impl<T1, T2, false, true>
{
    using type = T1;
};

template <class T1, class T2>
struct minimum_category_impl<T1, T2, true, true>
{
    static_assert(std::is_same<T1, T2>::value, "Iterator category types must be the same when they are equivalent.");
    using type = T1;
};

} // namespace detail

//
// Returns the minimum category type or fails to compile
// if T1 and T2 are unrelated.
//
template< class T1 = mpl::arg<1>, class T2 = mpl::arg<2> >
struct minimum_category
{
    static_assert(
        std::is_convertible<T1, T2>::value || std::is_convertible<T2, T1>::value,
        "Iterator category types must be related through convertibility.");

    using type = typename boost::iterators::detail::minimum_category_impl<
        T1,
        T2,
        std::is_convertible<T1, T2>::value,
        std::is_convertible<T2, T1>::value
    >::type;
};

template <>
struct minimum_category< mpl::arg<1>, mpl::arg<2> >
{
    template <class T1, class T2>
    struct apply : minimum_category<T1, T2>
    {};
};

} // namespace iterators
} // namespace boost

#endif // BOOST_ITERATOR_MINIMUM_CATEGORY_HPP_INCLUDED_
