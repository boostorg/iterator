#ifndef BOOST_ITERATOR_ADAPTORS_HPP
#define BOOST_ITERATOR_ADAPTORS_HPP

#include <boost/utility.hpp> // for prior
#include <boost/iterator.hpp>

namespace boost {

template <class Final, class V, class R, class P, class C, class D>
struct repository
    : iterator<C, V, D, P, R>

{
    typedef Final type;
    typedef V value_type;
    typedef R reference;
    typedef C iterator_category;
    typedef D difference_type;
};

template <class Base>
struct downcastable : Base
{
    typedef typename Base::type final_t;
 public:
    final_t& downcast() { return static_cast<final_t&>(*this); }
    const final_t& downcast() const { return static_cast<const final_t&>(*this); }
};

template <class Base>
struct iterator_comparisons : Base
{
};

template <class Base1, class Base2>
inline bool operator==(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return xb.downcast().equal(yb.downcast());
}

template <class Base1, class Base2>
inline bool operator!=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<D2,B2>& yb)
{
    return !xb.downcast().equal(yb.downcast());
}

template <class Base1, class Base2>
inline bool operator<(const iterator_comparisons<Base1>& xb,
                      const iterator_comparisons<Base2>& yb)
{
    return yb.downcast().distance_from(xb.downcast()) < 0;
}

template <class Base1, class Base2>
inline bool operator>(const iterator_comparisons<Base1>& xb,
                      const iterator_comparisons<Base2>& yb)
{
    return yb.downcast().distance_from(xb.downcast()) > 0;
}

template <class Base1, class Base2>
inline bool operator>=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return yb.downcast().distance_from(xb.downcast()) >= 0;
}

template <class Base1, class Base2>
inline bool operator<=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return yb.downcast().distance_from(xb.downcast()) <= 0;
}


template <class B>
struct iterator_arith :  B { };

template <class Base>
typename Base::final operator+(
    iterator_arith<Base> i, typename Base::distance n)
{
    return i.downcast() += n;
}

template <class Base>
typename Base::final operator+(
    typename Base::distance n, iterator_arith<Base> i)
{
    return i.downcast() += n;
}

template <class Base1, class Base2>
typename Base1::difference_type operator-(
    const iterator_arith<Base1>& i,
    const iterator_arith<Base2>& j)
{
    return j.downcast().distance_from(i.downcast());
}


template <class Final
          , class V, class R, class P, class C, class D
          , class B = iterator_arith<
                iterator_comparisons<
                     downcastable<repository<Final, V, R, P, C, D> > > >
         >
struct iterator_adaptor : B
{
    typedef V value_type;
    typedef R reference;
    typedef P pointer;
    typedef C iterator_category;
    typedef D difference_type;

    reference operator*() const
    { return downcast().dereference(); }
    //operator->() const { return detail::operator_arrow(*this, iterator_category()); }
    reference operator[](difference_type n) const
    { return *(*this + n); }
    Final& operator++()
    { downcast().increment(); return downcast(); }
    Final operator++(int)
    { Final tmp(downcast()); ++*this; return tmp; }
    Final& operator--()
    { downcast().decrement(); return downcast(); }
    Final operator--(int)
    { Final tmp(downcast()); --*this; return tmp; }
    Final& operator+=(difference_type n)
    { downcast().advance(n); return downcast(); }
    Final& operator-=(difference_type n)
    { downcast().advance(-n); return downcast(); }
    Final operator-(difference_type x) const
    { Final result(downcast()); return result -= x; }

    template <class Final2, class V2, class R2, class P2>
    bool equal(iterator_adaptor<Final2,V2,R2,P2,C,D> const& x) const
    {
        return this->downcast().base() == x.downcast().base();
    }
    
    void increment() { ++this->downcast().base(); }
    void decrement() { --this->downcast().base(); }
    
 private:
    using B::downcast;
};


template <class Base, 
          class V, class R, class P, class C, class D>
struct reverse_iterator
    : iterator_adaptor<
         reverse_iterator<Base, V, R, P, C, D>, V, R, P, C, D
      >
{
    typedef iterator_adaptor<reverse_iterator<Base, V, R, P, C, D>, V, R, P, C, D> super;
    
//  friend class super;
    // hmm, I don't like the next two lines
//  template <class Der> friend struct iterator_comparisons;
//  template <class Der, class Dist> friend struct iterator_arith;
    
 public:
    reverse_iterator(const Base& x) : m_base(x) { }

    reverse_iterator() { }

    Base const& base() const { return m_base; }
    
    template <class B2, class V2, class R2, class P2>
    reverse_iterator(const reverse_iterator<B2,V2,R2,P2,C,D>& y)
        : m_base(y.m_base) { }


    struct state : default_iterator_policies
    {
    };
    
    typename super::reference dereference() const { return *boost::prior(m_base); }
    
    void increment() { --m_base; }
    void decrement() { ++m_base; }
    void advance(typename super::difference_type n)
    {
        m_base -= n;
    }

    template <class B2, class V2, class R2, class P2>
    typename super::difference_type
    distance_from(const reverse_iterator<B2,V2,R2,P2,C,D>& y) const
    {
        return m_base - y.m_base;
    }
 private:
    Base m_base;
};




} // namespace boost


#endif // BOOST_ITERATOR_ADAPTORS_HPP
