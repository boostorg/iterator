// (C) Copyright Jeremy Siek 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_ITERATOR_ARCHETYPES_HPP
#define BOOST_ITERATOR_ARCHETYPES_HPP

#include <boost/iterator/iterator_categories.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_cv.hpp>

#include <cstddef>

namespace boost
{

  template <class Value, class AccessCategory>
  struct access_archetype;

  template <class Derived, class Value, class AccessCategory, class TraversalCategory>
  struct traversal_archetype;

  namespace detail {

    template <class T>
    struct assign_proxy
    {
      assign_proxy& operator=(T);
    };

    template <class T>
    struct read_write_proxy :
      assign_proxy<T>
    {
      operator T();
    };

    template <class T>
    struct arrow_proxy
    {
      T const* operator->() const;
    };

    struct no_operator_brackets {};

    template <class ValueType>
    struct readable_operator_brackets
    {
      ValueType operator[](std::ptrdiff_t n) const;
    };

    template <class ValueType>
    struct writable_operator_brackets
    {
      read_write_proxy<ValueType> operator[](std::ptrdiff_t n) const;
    };

    template <class Value, class AccessCategory, class TraversalCategory>
    struct operator_brackets :
      mpl::if_< is_tag<random_access_traversal_tag, TraversalCategory>,
                 mpl::if_< is_tag<writable_iterator_tag, AccessCategory>,
                           writable_operator_brackets< Value >,
                           mpl::if_< is_tag<readable_iterator_tag, AccessCategory>,
                                     readable_operator_brackets<Value>,
                                     no_operator_brackets > >,
                 no_operator_brackets >::type
    {
    };

    template <class Derived, class Value, class TraversalCategory>
    struct traversal_archetype_;

    template<class Derived, class Value>
    struct traversal_archetype_<Derived, Value, incrementable_iterator_tag>
    {
      typedef void difference_type;

      Derived& operator++();
      Derived  operator++(int) const;
    };

    template<class Derived, class Value>
    struct traversal_archetype_<Derived, Value, single_pass_iterator_tag>
      : public equality_comparable< traversal_archetype_<Derived, Value, single_pass_iterator_tag> >,
        public traversal_archetype_<Derived, Value, incrementable_iterator_tag>
    {
    };

    template <class Derived, class Value>
    bool operator==(traversal_archetype_<Derived, Value, single_pass_iterator_tag> const&,
                    traversal_archetype_<Derived, Value, single_pass_iterator_tag> const&);

    template<class Derived, class Value>
    struct traversal_archetype_<Derived, Value, forward_traversal_tag>
      : public traversal_archetype_<Derived, Value, single_pass_iterator_tag>
    {
      typedef std::ptrdiff_t difference_type;
    };

    template<class Derived, class Value>
    struct traversal_archetype_<Derived, Value, bidirectional_traversal_tag>
      : public traversal_archetype_<Derived, Value, forward_traversal_tag>
    {
      Derived& operator--();
      Derived  operator--(int) const;
    };

    template<class Derived, class Value>
    struct traversal_archetype_<Derived, Value, random_access_traversal_tag>
      : public partially_ordered< traversal_archetype_<Derived, Value, random_access_traversal_tag> >,
        public traversal_archetype_<Derived, Value, bidirectional_traversal_tag> 
    {
      Derived& operator+=(std::ptrdiff_t);
      Derived& operator-=(std::ptrdiff_t);
    };

    template <class Derived, class Value>
    Derived& operator+(traversal_archetype_<Derived, Value, random_access_traversal_tag> const&,
                       std::ptrdiff_t);

    template <class Derived, class Value>
    Derived& operator+(std::ptrdiff_t,
                       traversal_archetype_<Derived, Value, random_access_traversal_tag> const&);

    template <class Derived, class Value>
    Derived& operator-(traversal_archetype_<Derived, Value, random_access_traversal_tag> const&,
                       std::ptrdiff_t);

    template <class Derived, class Value>
    std::ptrdiff_t operator-(traversal_archetype_<Derived, Value, random_access_traversal_tag> const&,
                             traversal_archetype_<Derived, Value, random_access_traversal_tag> const&);

    template <class Derived, class Value>
    bool operator<(traversal_archetype_<Derived, Value, random_access_traversal_tag> const&,
                   traversal_archetype_<Derived, Value, random_access_traversal_tag> const&);

    struct bogus_type;

    template <class Value>
    struct convertible_type
      : mpl::if_< is_const<Value>,
                  typename remove_const<Value>::type,
                  bogus_type >
    {};

  } // namespace detail


  template <class Value, class AccessCategory>
  struct access_archetype;

  template <class Value>
  struct access_archetype<Value, readable_iterator_tag>
  {
    typedef typename remove_cv<Value>::type value_type;
    typedef Value                           reference;
    typedef Value*                          pointer;

    value_type operator*() const;

    detail::arrow_proxy<Value> operator->() const;
  };

  template <class Value>
  struct access_archetype<Value, writable_iterator_tag>
  {
    BOOST_STATIC_ASSERT((!is_const<Value>::value));

    typedef void value_type;
    typedef void reference;
    typedef void pointer;

    detail::assign_proxy<Value> operator*() const;
  };

  template <class Value>
  struct access_archetype<Value, readable_writable_iterator_tag> :
    public virtual access_archetype<Value, readable_iterator_tag>
  {
    typedef detail::read_write_proxy<Value>    reference;

    detail::read_write_proxy<Value> operator*() const;
  };

  template <class Value>
  struct access_archetype<Value, readable_lvalue_iterator_tag> :
    public virtual access_archetype<Value, readable_iterator_tag>
  {
    typedef Value&    reference;

    Value& operator*() const;
    Value* operator->() const;
  };

  template <class Value>
  struct access_archetype<Value, writable_lvalue_iterator_tag>
    : public virtual access_archetype<Value, readable_lvalue_iterator_tag>
  {
    BOOST_STATIC_ASSERT((!is_const<Value>::value));
  };

  template <class Derived, class Value, class AccessCategory, class TraversalCategory>
  struct traversal_archetype 
    : detail::operator_brackets< typename remove_cv<Value>::type,
                                 AccessCategory,
                                 TraversalCategory >,
      detail::traversal_archetype_<Derived, Value, TraversalCategory>
  {
  };

  template <class Value, class AccessCategory, class TraversalCategory>
  struct iterator_archetype
    : public traversal_archetype<iterator_archetype<Value, AccessCategory, TraversalCategory>, Value, AccessCategory, TraversalCategory>,
      public access_archetype<Value, AccessCategory>
  {
    typedef iterator_tag<AccessCategory, TraversalCategory> iterator_category; 

    iterator_archetype();
    iterator_archetype(iterator_archetype const&);

    iterator_archetype& operator=(iterator_archetype const&);

    // Optional conversion from mutable
    // iterator_archetype(iterator_archetype<typename detail::convertible_type<Value>::type, AccessCategory, TraversalCategory> const&);
  };

} // namespace boost


#endif // BOOST_ITERATOR_ARCHETYPES_HPP
