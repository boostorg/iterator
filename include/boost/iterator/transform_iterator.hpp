// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#ifndef BOOST_TRANSFORM_ITERATOR_23022003THW_HPP
#define BOOST_TRANSFORM_ITERATOR_23022003THW_HPP

#include <boost/function.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/detail/enable_if.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>

namespace boost
{
  template <class UnaryFunction, class Iterator, class Reference = use_default, class Value = use_default>
  class transform_iterator;

  namespace detail 
  {

    // Given the transform iterator's transformation and iterator, this
    // is the type used as its traits.
    template <class UnaryFunction, class Iterator, class Reference, class Value>
    struct transform_iterator_base
    {
    private:

      // transform_iterator does not support writable/swappable iterators
#if !BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
      BOOST_STATIC_ASSERT((is_tag< readable_iterator_tag, typename access_category<Iterator>::type >::value));
#endif
 
      typedef typename UnaryFunction::result_type result_type;

      typedef typename remove_reference< result_type >::type cv_value_type;

      typedef typename mpl::if_< 
          is_reference< result_type >
        , typename mpl::if_<
              is_const< cv_value_type >
            , readable_lvalue_iterator_tag
            , writable_lvalue_iterator_tag
          >::type
        , readable_iterator_tag
      >::type maximum_access_tag;
  
      typedef typename minimum_category<
          maximum_access_tag
        , typename access_category<Iterator>::type
      >::type access_category;

    public:
      typedef iterator_adaptor<
          transform_iterator<UnaryFunction, Iterator>
        , Iterator
        , cv_value_type  
        , iterator_tag<
             access_category
           , typename traversal_category<Iterator>::type
          >
        , result_type  
      > type;
    };

  }

  template <class UnaryFunction, class Iterator, class Reference, class Value>
  class transform_iterator
    : public detail::transform_iterator_base<UnaryFunction, Iterator, Reference, Value>::type
  {
    typedef typename
    detail::transform_iterator_base<UnaryFunction, Iterator, Reference, Value>::type
    super_t;

    friend class iterator_core_access;

  public:
    transform_iterator() { }

    transform_iterator(Iterator const& x, UnaryFunction f)
      : super_t(x), m_f(f) { }

    template<class OtherIterator>
    transform_iterator(
          transform_iterator<UnaryFunction, OtherIterator> const& t
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
    )
      : super_t(t.base()), m_f(t.functor()) {}

    UnaryFunction functor() const
      { return m_f; }

  private:
    typename super_t::reference dereference() const
    { return m_f(*this->base()); }

    // Probably should be the initial base class so it can be
    // optimized away via EBO if it is an empty class.
    UnaryFunction m_f;
  };

  template <class UnaryFunctionObject, class Iterator>
  transform_iterator<UnaryFunctionObject, Iterator> make_transform_iterator(Iterator it, UnaryFunctionObject fun)
  {
    return transform_iterator<UnaryFunctionObject, Iterator>(it, fun);
  }

  namespace detail {

    template <class Function>
    struct is_unary :
      mpl::bool_<(function_traits<Function>::arity == 1)>
    {};

    template <class T>
    struct is_unary_function :
      mpl::apply_if< is_function<T>
                     , is_unary<T>
                     , mpl::bool_<false>
      >::type
    {};

  }

  //
  // ToDo: Think twice wether enable_if is better than an
  // static assert. Currently we get convoluted error messages
  // from the above overload for any pointer that is not a
  // pointer to a unary function.
  //
  template <class UnaryFunction, class Iterator>
  typename detail::enable_if<
    detail::is_unary_function<UnaryFunction>
    , transform_iterator< function<UnaryFunction>, Iterator>
  >::type
  make_transform_iterator(Iterator it, UnaryFunction* fun)
  {
    return make_transform_iterator(it, function<UnaryFunction>(fun));
  }

} // namespace boost

#endif // BOOST_TRANSFORM_ITERATOR_23022003THW_HPP
