// (C) Copyright Jeremy Siek 2001.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Revision History:

// 27 Feb 2001   Jeremy Siek
//      Initial checkin.

#ifndef BOOST_ITERATOR_FUNCTION_OUTPUT_ITERATOR_HPP
#define BOOST_ITERATOR_FUNCTION_OUTPUT_ITERATOR_HPP

#include <iterator>
#include <boost/config.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_cv.hpp>
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
#include <boost/type_traits/remove_reference.hpp>
#endif

namespace boost {
namespace iterators {

  template <class UnaryFunction>
  class function_output_iterator {
  private:
    typedef function_output_iterator self;

  public:
    typedef std::output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef std::ptrdiff_t      difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit function_output_iterator() {}

    explicit function_output_iterator(const UnaryFunction& f)
      : m_f(f) {}

    self& operator*() { return *this; }
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
    template <class T>
    self& operator=(const T& value) {
      m_f(value);
      return *this;
    }
#else
    template <class T>
    self& operator=(T&& value) {
      m_f(static_cast<T &&>(value));
      return *this;
    }
#endif
    self& operator++() { return *this; }
    self& operator++(int) { return *this; }

  private:
    UnaryFunction m_f;
  };

  template <class UnaryFunction>
  inline function_output_iterator<UnaryFunction>
  make_function_output_iterator(const UnaryFunction& f = UnaryFunction()) {
    return function_output_iterator<UnaryFunction>(f);
  }

} // namespace iterators

using iterators::function_output_iterator;
using iterators::make_function_output_iterator;

} // namespace boost

#endif // BOOST_ITERATOR_FUNCTION_OUTPUT_ITERATOR_HPP
