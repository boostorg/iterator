#ifndef BOOST_ITERATOR_ADAPTORS_HPP
#define BOOST_ITERATOR_ADAPTORS_HPP

#include <boost/utility.hpp> // for prior
#include <boost/iterator.hpp>
#include <boost/iterator/iterator_categories.hpp>

namespace boost {

template <class Final, class V, class R, class P, class C, class D>
struct repository : iterator<C, V, D, P, R>

{
    typedef Final final;
};

template <class Base>
struct downcastable : Base
{
    typedef typename Base::final final_t;
public:
    final_t& self() { return static_cast<final_t&>(*this); }
    const final_t& self() const { return static_cast<const final_t&>(*this); }
};

template <class Base>
struct iterator_comparisons : Base
{
};

template <class Base1, class Base2>
inline bool operator==(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return xb.self().equal(yb.self());
}

template <class Base1, class Base2>
inline bool operator!=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return !xb.self().equal(yb.self());
}

template <class Base1, class Base2>
inline bool operator<(const iterator_comparisons<Base1>& xb,
                      const iterator_comparisons<Base2>& yb)
{
    return xb.self().distance_to(yb.self()) < 0;
}

template <class Base1, class Base2>
inline bool operator>(const iterator_comparisons<Base1>& xb,
                      const iterator_comparisons<Base2>& yb)
{
    return xb.self().distance_to(yb.self()) > 0;
}

template <class Base1, class Base2>
inline bool operator>=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return xb.self().distance_to(yb.self()) >= 0;
}

template <class Base1, class Base2>
inline bool operator<=(const iterator_comparisons<Base1>& xb,
                       const iterator_comparisons<Base2>& yb)
{
    return xb.self().distance_to(yb.self()) <= 0;
}


template <class B>
struct iterator_arith :  B { };

template <class Base>
typename Base::final operator+(
    const iterator_arith<Base>& i, // pass by ref, not value to avoid slicing -JGS
    typename Base::difference_type n)
{
    typename Base::final tmp(i.self());
    return tmp += n;
}

template <class Base>
typename Base::final operator+(
    typename Base::difference_type n,  
    const iterator_arith<Base>& i) // pass by ref, not value to avoid slicing -JGS
{
    typename Base::final tmp(i.self());
    return tmp += n;
}

template <class Base1, class Base2>
typename Base1::difference_type operator-(
    const iterator_arith<Base1>& i,
    const iterator_arith<Base2>& j)
{
    return i.self().distance_to(i.self());
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
    { return self().dereference(); }
    //operator->() const { return detail::operator_arrow(*this, iterator_category()); }
    reference operator[](difference_type n) const
    { return *(*this + n); }
    Final& operator++()
    { self().increment(); return self(); }
    Final operator++(int)
    { Final tmp(self()); ++*this; return tmp; }
    Final& operator--()
    { self().decrement(); return self(); }
    Final operator--(int)
    { Final tmp(self()); --*this; return tmp; }
    Final& operator+=(difference_type n)
    { self().advance(n); return self(); }
    Final& operator-=(difference_type n)
    { self().advance(-n); return self(); }
    Final operator-(difference_type x) const
    { Final result(self()); return result -= x; }

    template <class Final2, class V2, class R2, class P2, class B2>
    bool equal(iterator_adaptor<Final2,V2,R2,P2,C,D,B2> const& x) const
    {
        return self().base() == x.self().base();
    }
    void advance(difference_type n)
    {
        self().base() += n;
    }

    reference dereference() const { return *self().base(); }  
    
    void increment() { ++self().base(); }
    void decrement() { --self().base(); }

    template <class Final2, class B2, class V2, class R2, class P2>
    difference_type
    distance_to(const iterator_adaptor<Final2,V2,R2,P2,C,D,B2>& y) const
    {
        return self().base() - y.self().base();//?
    }
    
 private:
    using B::self;
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


    typename super::reference dereference() const { return *boost::prior(m_base); }
    
    void increment() { --m_base; }
    void decrement() { ++m_base; }
    void advance(typename super::difference_type n)
    {
        m_base -= n;
    }

    template <class B2, class V2, class R2, class P2>
    typename super::difference_type
    distance_to(const reverse_iterator<B2,V2,R2,P2,C,D>& y) const
    {
        return y.m_base - m_base;
    }
 private:
    Base m_base;
};

template <class AdaptableUnaryFunction, class Base>
struct transform_iterator
    : iterator_adaptor<
         transform_iterator<AdaptableUnaryFunction, Base>, 
         typename AdaptableUnaryFunction::result_type,
         typename AdaptableUnaryFunction::result_type, 
         typename AdaptableUnaryFunction::result_type*, 
         iterator_tag<readable_iterator_tag,
                      typename traversal_category<Base>::type>, 
         typename detail::iterator_traits<Base>::difference_type
      >
{
 public: // types
  typedef typename AdaptableUnaryFunction::result_type value_type;

 public: // member functions
  transform_iterator() { }

  transform_iterator(const Base& x, AdaptableUnaryFunction f)
    : m_base(x), m_f(f) { }

  value_type dereference() const { return m_f(*m_base); }

  template <class F2, class B2>
  transform_iterator(const transform_iterator<F2,B2>& y)
	: m_base(y.m_base), m_f(y.m_f) { }

  Base& base() { return m_base; }
  Base const& base() const { return m_base; }
    
private:
  Base m_base;
  AdaptableUnaryFunction m_f;
};

} // namespace boost


#endif // BOOST_ITERATOR_ADAPTORS_HPP
