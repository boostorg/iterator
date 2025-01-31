// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_ITERATOR_ADAPTOR_23022003THW_HPP
#define BOOST_ITERATOR_ADAPTOR_23022003THW_HPP

#include <type_traits>

#include <boost/core/use_default.hpp>

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>

#include <boost/iterator/detail/config_def.hpp>

namespace boost {
namespace iterators {

// Used as a default template argument internally, merely to
// indicate "use the default", this can also be passed by users
// explicitly in order to specify that the default should be used.
using boost::use_default;

namespace detail {

//
// Result type used in enable_if_convertible meta function.
// This can be an incomplete type, as only pointers to
// enable_if_convertible< ... >::type are used.
// We could have used void for this, but conversion to
// void* is just too easy.
//
struct enable_type;

} // namespace detail

//
// enable_if for use in adapted iterators constructors.
//
// In order to provide interoperability between adapted constant and
// mutable iterators, adapted iterators will usually provide templated
// conversion constructors of the following form
//
// template <class BaseIterator>
// class adapted_iterator :
//   public iterator_adaptor< adapted_iterator<Iterator>, Iterator >
// {
// public:
//
//   ...
//
//   template <class OtherIterator>
//   adapted_iterator(
//       OtherIterator const& it
//     , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0);
//
//   ...
// };
//
// enable_if_convertible is used to remove those overloads from the overload
// set that cannot be instantiated. For all practical purposes only overloads
// for constant/mutable interaction will remain. This has the advantage that
// meta functions like boost::is_convertible do not return false positives,
// as they can only look at the signature of the conversion constructor
// and not at the actual instantiation.
//
// enable_if_interoperable can be safely used in user code. It falls back to
// always enabled for compilers that don't support enable_if or is_convertible.
// There is no need for compiler specific workarounds in user code.
//
// The operators implementation relies on boost::is_convertible not returning
// false positives for user/library defined iterator types. See comments
// on operator implementation for consequences.
//
template< typename From, typename To >
struct enable_if_convertible :
    public std::enable_if<
        std::is_convertible< From, To >::value,
        boost::iterators::detail::enable_type
    >
{};

//
// Default template argument handling for iterator_adaptor
//
namespace detail {

// If T is use_default, return the result of invoking
// DefaultNullaryFn, otherwise return T.
template< typename T, typename DefaultNullaryFn >
struct ia_dflt_help
{
    using type = T;
};

template< typename DefaultNullaryFn >
struct ia_dflt_help< use_default, DefaultNullaryFn >
{
    using type = typename DefaultNullaryFn::type;
};

template< typename T, typename DefaultNullaryFn >
using ia_dflt_help_t = typename ia_dflt_help< T, DefaultNullaryFn >::type;

// If T is use_default, return the result of invoking
// DefaultNullaryFn, otherwise - of NondefaultNullaryFn.
template< typename T, typename DefaultNullaryFn, typename NondefaultNullaryFn >
struct ia_eval_if_default
{
    using type = typename NondefaultNullaryFn::type;
};

template< typename DefaultNullaryFn, typename NondefaultNullaryFn >
struct ia_eval_if_default< use_default, DefaultNullaryFn, NondefaultNullaryFn >
{
    using type = typename DefaultNullaryFn::type;
};

template< typename T, typename DefaultNullaryFn, typename NondefaultNullaryFn >
using ia_eval_if_default_t = typename ia_eval_if_default< T, DefaultNullaryFn, NondefaultNullaryFn >::type;

// A metafunction which computes an iterator_adaptor's base class,
// a specialization of iterator_facade.
template<
    typename Derived,
    typename Base,
    typename Value,
    typename Traversal,
    typename Reference,
    typename Difference
>
using iterator_adaptor_base_t = iterator_facade<
    Derived,

#ifdef BOOST_ITERATOR_REF_CONSTNESS_KILLS_WRITABILITY
    detail::ia_dflt_help_t<
        Value,
        detail::ia_eval_if_default<
            Reference,
            iterator_value< Base >,
            std::remove_reference< Reference >
        >
    >,
#else
    detail::ia_dflt_help_t<
        Value,
        iterator_value< Base >
    >,
#endif

    detail::ia_dflt_help_t<
        Traversal,
        iterator_traversal< Base >
    >,

    detail::ia_dflt_help_t<
        Reference,
        detail::ia_eval_if_default<
            Value,
            iterator_reference< Base >,
            std::add_lvalue_reference< Value >
        >
    >,

    detail::ia_dflt_help_t<
        Difference,
        iterator_difference< Base >
    >
>;

} // namespace detail

//
// Iterator Adaptor
//
// The parameter ordering changed slightly with respect to former
// versions of iterator_adaptor The idea is that when the user needs
// to fiddle with the reference type it is highly likely that the
// iterator category has to be adjusted as well.  Any of the
// following four template arguments may be omitted or explicitly
// replaced by use_default.
//
//   Value - if supplied, the value_type of the resulting iterator, unless
//      const. If const, a conforming compiler strips constness for the
//      value_type. If not supplied, iterator_traits<Base>::value_type is used
//
//   Category - the traversal category of the resulting iterator. If not
//      supplied, iterator_traversal<Base>::type is used.
//
//   Reference - the reference type of the resulting iterator, and in
//      particular, the result type of operator*(). If not supplied but
//      Value is supplied, Value& is used. Otherwise
//      iterator_traits<Base>::reference is used.
//
//   Difference - the difference_type of the resulting iterator. If not
//      supplied, iterator_traits<Base>::difference_type is used.
//
template<
    typename Derived,
    typename Base,
    typename Value        = use_default,
    typename Traversal    = use_default,
    typename Reference    = use_default,
    typename Difference   = use_default
>
class iterator_adaptor :
    public detail::iterator_adaptor_base_t<
        Derived, Base, Value, Traversal, Reference, Difference
    >
{
    friend class iterator_core_access;

protected:
    using super_t = detail::iterator_adaptor_base_t<
        Derived, Base, Value, Traversal, Reference, Difference
    >;

public:
    using base_type = Base;

    iterator_adaptor() = default;

    explicit iterator_adaptor(Base const& iter) :
        m_iterator(iter)
    {
    }

    base_type const& base() const { return m_iterator; }

protected:
    // for convenience in derived classes
    using iterator_adaptor_ = iterator_adaptor< Derived, Base, Value, Traversal, Reference, Difference >;

    //
    // lvalue access to the Base object for Derived
    //
    Base& base_reference() { return m_iterator; }
    Base const& base_reference() const { return m_iterator; }

private:
    //
    // Core iterator interface for iterator_facade.  This is private
    // to prevent temptation for Derived classes to use it, which
    // will often result in an error.  Derived classes should use
    // base_reference(), above, to get direct access to m_iterator.
    //
    typename super_t::reference dereference() const { return *m_iterator; }

    template< typename OtherDerived, typename OtherIterator, typename V, typename C, typename R, typename D >
    bool equal(iterator_adaptor< OtherDerived, OtherIterator, V, C, R, D > const& x) const
    {
        // Maybe readd with same_distance
        //           BOOST_STATIC_ASSERT(
        //               (detail::same_category_and_difference<Derived,OtherDerived>::value)
        //               );
        return m_iterator == x.base();
    }

    using my_traversal = typename iterator_category_to_traversal< typename super_t::iterator_category >::type;

    void advance(typename super_t::difference_type n)
    {
        static_assert(detail::is_traversal_at_least< my_traversal, random_access_traversal_tag >::value,
            "Iterator must support random access traversal.");
        m_iterator += n;
    }

    void increment() { ++m_iterator; }

    void decrement()
    {
        static_assert(detail::is_traversal_at_least< my_traversal, bidirectional_traversal_tag >::value,
            "Iterator must support bidirectional traversal.");
        --m_iterator;
    }

    template< typename OtherDerived, typename OtherIterator, typename V, typename C, typename R, typename D >
    typename super_t::difference_type distance_to(iterator_adaptor< OtherDerived, OtherIterator, V, C, R, D > const& y) const
    {
        static_assert(detail::is_traversal_at_least< my_traversal, random_access_traversal_tag >::value,
            "Super iterator must support random access traversal.");
        // Maybe readd with same_distance
        //           BOOST_STATIC_ASSERT(
        //               (detail::same_category_and_difference<Derived,OtherDerived>::value)
        //               );
        return y.base() - m_iterator;
    }

private: // data members
    Base m_iterator;
};

} // namespace iterators

using iterators::iterator_adaptor;
using iterators::enable_if_convertible;

} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_ITERATOR_ADAPTOR_23022003THW_HPP
