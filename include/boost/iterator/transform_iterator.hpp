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

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>

namespace boost
{
  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator;

  namespace detail 
  {
    // Given the transform iterator's transformation and iterator, this
    // is the type used as its traits.
    template <class AdaptableUnaryFunction, class Iterator>
    struct transform_iterator_base
    {
        typedef iterator_adaptor<
            transform_iterator<AdaptableUnaryFunction, Iterator>
          , Iterator
          , typename AdaptableUnaryFunction::result_type  
          , iterator_tag<
                readable_iterator_tag
              , typename traversal_category<Iterator>::type
            >
          , typename AdaptableUnaryFunction::result_type  
        > type;
    };
  }

  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator
    : public detail::transform_iterator_base<AdaptableUnaryFunction, Iterator>::type
  {
    typedef typename
      detail::transform_iterator_base<AdaptableUnaryFunction, Iterator>::type
    super_t;

    friend class iterator_core_access;

  public:
    transform_iterator() { }

    transform_iterator(Iterator const& x, AdaptableUnaryFunction f)
      : super_t(x), m_f(f) { }

    template<class OtherIterator>
    transform_iterator(
          transform_iterator<AdaptableUnaryFunction, OtherIterator> const& t
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
    )
      : super_t(t.base()), m_f(t.functor()) {}

    AdaptableUnaryFunction functor() const
      { return m_f; }

  private:
    typename super_t::value_type dereference() const
      { return m_f(super_t::dereference()); }

    // Probably should be the initial base class so it can be
    // optimized away via EBO if it is an empty class.
    AdaptableUnaryFunction m_f;
  };

} // namespace boost

#endif // BOOST_TRANSFORM_ITERATOR_23022003THW_HPP
