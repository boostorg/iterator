//  (C) Copyright David Abrahams 2000. Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

#ifndef BOOST_ITERATOR_ADAPTOR_DWA053000_HPP_
# define BOOST_ITERATOR_ADAPTOR_DWA053000_HPP_

// MSVC complains about the wrong stuff unless you disable this. We should add
// this to config.hpp

#include <boost/operators.hpp>
#include <iterator>

namespace boost {

// Just a "type envelope"; works around some MSVC deficiencies.
template <class T>
struct type {};

// Default policies for wrapped iterators. You can use this as a base
// class if you want to customize particular policies.
struct default_iterator_policies
{
    template <class Reference, class Iterator>
    static Reference dereference(type<Reference>, const Iterator& x)
        { return *x; }

    template <class Iterator>
    static void increment(Iterator& x)
        { ++x; }

    template <class Iterator>
    static void decrement(Iterator& x)
        { --x; }

    template <class Iterator, class DifferenceType>
    static void advance(Iterator& x, DifferenceType n)
        { x += n; }

    template <class Difference, class Iterator1, class Iterator2>
    static Difference distance(type<Difference>, Iterator1& x, Iterator2& y)
        { return y - x; }

    template <class Iterator1, class Iterator2>
    static bool equal(Iterator1& x, Iterator2& y)
        { return x == y; }

    template <class Iterator1, class Iterator2>
    static bool less(Iterator1& x, Iterator2& y)
        { return x < y; }
};

// iterator_adaptor - A generalized adaptor around an existing iterator, which is itself an iterator
//
//      Iterator - the iterator type being wrapped.
//
//      Policies - a set of policies determining how the resulting iterator
//      works.
//
//      NonconstIterator - the corresponding non-const iterator type for
//      Iterator, if any. You don't need to supply this if you are not make a
//      const/non-const iterator pair.
//
//      Traits - a class satisfying the same requirements as a specialization of
//      std::iterator_traits for the resulting iterator.
//
template <class Iterator, class Policies, class NonconstIterator = Iterator, class Traits = std::iterator_traits<Iterator> >
struct iterator_adaptor
    : std::iterator<typename Traits::iterator_category, typename Traits::value_type, typename Traits::difference_type, typename Traits::pointer, typename Traits::reference>
{
    typedef iterator_adaptor<Iterator, Policies, NonconstIterator, Traits> Self;
public:
    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::reference reference;
    typedef typename Traits::iterator_category iterator_category;

    iterator_adaptor(const Iterator& impl)
        : m_impl(impl) {}

    template <class OtherTraits>
    iterator_adaptor(const iterator_adaptor<NonconstIterator, Policies, NonconstIterator, OtherTraits>& rhs)
        : m_impl(rhs.m_impl) {}

    template <class OtherTraits>
    Self& operator=(const iterator_adaptor<NonconstIterator, Policies, NonconstIterator, OtherTraits>& rhs)
        { m_impl = rhs.m_impl; return *this; }
    
    reference operator*() const {
        return Policies::dereference(type<reference>(), m_impl);
    }

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning( disable : 4284 )
#endif

    pointer operator->() const
        { return &*this; }

#ifdef _MSC_VER
# pragma warning(pop)
#endif

    reference operator[](difference_type n)
        { return *(*this + n); }
    
    Self& operator++() {
        Policies::increment(m_impl);
        return *this;
    }

    Self& operator++(int) { Self tmp(*this); ++*this; return tmp; }
    
    Self& operator--() {
        Policies::decrement(m_impl);
        return *this;
    }
    
    Self& operator--(int) { Self tmp(*this); --*this; return tmp; }

    Self& operator+=(difference_type n) {
        Policies::advance(m_impl, n);
        return *this;
    }
  
    Self& operator-=(difference_type n) {
        Policies::advance(m_impl, -n);
        return *this;
    }

private:
    typedef Policies policies_type;
public: // too many compilers have trouble when this is private.
    Iterator m_impl;
};

template <class Iterator, class Policies, class NonconstIterator, class Traits>
iterator_adaptor<Iterator,Policies,NonconstIterator,Traits>
operator-(iterator_adaptor<Iterator,Policies,NonconstIterator,Traits> p, const typename Traits::difference_type x)
{
    return p -= x;
}

template <class Iterator, class Policies, class NonconstIterator, class Traits>
iterator_adaptor<Iterator,Policies,NonconstIterator,Traits>
operator+(iterator_adaptor<Iterator,Policies,NonconstIterator,Traits> p, const typename Traits::difference_type x)
{
    return p += x;
}

template <class Iterator, class Policies, class NonconstIterator, class Traits>
iterator_adaptor<Iterator,Policies,NonconstIterator,Traits>
operator+(const typename Traits::difference_type x, iterator_adaptor<Iterator,Policies,NonconstIterator,Traits> p)
{
    return p += x;
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
typename Traits1::difference_type operator-(
    const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x,
    const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y )
{
    typedef typename Traits1::difference_type difference_type;
    return Policies::distance(type<difference_type>(), y.m_impl, x.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator==(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x, const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) {
    return Policies::equal(x.m_impl, y.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator<(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x, const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) {
    return Policies::less(x.m_impl, y.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator>(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x,
          const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) { 
    return Policies::less(y.m_impl, x.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator>=(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x, const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) {
    return !Policies::less(x.m_impl, y.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator<=(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x,
           const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) {
    return !Policies::less(y.m_impl, x.m_impl);
}

template <class Iterator1, class Iterator2, class Policies, class NonconstIterator, class Traits1, class Traits2>
inline bool 
operator!=(const iterator_adaptor<Iterator1,Policies,NonconstIterator,Traits1>& x, 
           const iterator_adaptor<Iterator2,Policies,NonconstIterator,Traits2>& y) {
    return !Policies::equal(x.m_impl, y.m_impl);
}

}

#endif
