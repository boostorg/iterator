#ifndef BOOST_ITERATOR_ADAPTORS_HPP
#define BOOST_ITERATOR_ADAPTORS_HPP

#include <boost/config.hpp> // for prior
#include <boost/static_assert.hpp>
#include <boost/utility.hpp> // for prior
#include <boost/iterator.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/mpl/aux_/has_xxx.hpp>
#include <boost/mpl/logical/or.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include "boost/type_traits/detail/bool_trait_def.hpp"

#if BOOST_WORKAROUND(BOOST_MSVC, <= 1301) || BOOST_WORKAROUND(__GNUC__, <= 2 && __GNUC_MINOR__ <= 95) || BOOST_WORKAROUND(__MWERKS__, <= 0x3000)
# define BOOST_NO_SFINAE // "Substitution Failure Is Not An Error not implemented"
#endif 

namespace boost {

  namespace detail {

    template<bool>
    struct enabled
    {
      template<typename T>
      struct base
      {
        typedef T type;
      };
    };
    
    template<>
    struct enabled<false>
    {
      template<typename T>
      struct base
      {
#ifdef BOOST_NO_SFINAE
        // Disable enable if for MSVC
        typedef T type;

        // This would give a nice error messages containing
        // invalid overlaod, but has the big disadvantage that
        // there is no reference to user code.
        // struct invalid_overload;
        // typedef invalid_overload type;
#endif
      };
    };

    struct enable_type {};

    template <typename A, typename B>
    struct is_interoperable :
      mpl::logical_or< is_convertible< A, B >,
                       is_convertible< B, A > >
    {
    };

    template <class Facade1,
              class Facade2,
              class Return>
    struct enable_if_interoperable :
      enabled< is_interoperable<Facade1, Facade2>::value >::template base<Return>
    {
#if BOOST_WORKAROUND(BOOST_MSVC, <=1200)
      typedef typename enabled< is_interoperable<Facade1, Facade2>::value >::template base<Return>::type type;
#endif
    };

  } // namespace detail


  template<typename From,
           typename To>
  struct enable_if_convertible :
    detail::enabled< is_convertible<From, To>::value >::template base<detail::enable_type>
  {
#if BOOST_WORKAROUND(BOOST_MSVC, <=1200)
    typedef typename detail::enabled< is_convertible<From, To>::value >::template base<detail::enable_type>::type type;
#endif
  };

  //
  // Helper class for granting access to the iterator core interface.
  //
  // The simple core interface is used by iterator_facade. The core
  // interface should not be made public so that it does not clutter
  // the public interface of user defined iterators. Instead iterator_core_access
  // should be made friend so that iterator_facade can access the core
  // interface through iterator_core_access.
  //
  struct iterator_core_access
  {
    template <class Facade>
    static typename Facade::reference dereference(Facade const& f)
    {
      return f.dereference();
    }

    template <class Facade>
    static void increment(Facade& f)
    {
      f.increment();
    }

    template <class Facade>
    static void decrement(Facade& f)
    {
      f.decrement();
    }

    template <class Facade1, class Facade2>
    static bool equal(Facade1 const& f1, Facade2 const& f2, Facade1* = 0, Facade2* = 0)
    {
      return f1.equal(f2);
    }

    template <class Facade>
    static void advance(Facade& f, typename Facade::difference_type n)
    {
      f.advance(n);
    }

    template <class Facade1, class Facade2>
    static typename Facade1::difference_type distance_to(Facade1 const& f1,
                                                         Facade2 const& f2,
                                                         Facade1* = 0,
                                                         Facade2* = 0)
    {
      return f1.distance_to(f2);
    }

  };

  template <class Derived,
            class V,
            class R,
            class P,
            class C,
            class D>
  class repository :
    public iterator<C, V, D, P, R>
  {
  public:
    typedef Derived derived_t;
  };

  template <class Base>
  class downcastable :
    public Base
  {
  public:
    typename Base::derived_t& derived()
    {
      return static_cast<typename Base::derived_t&>(*this);
    }

    typename Base::derived_t const& derived() const 
    {
      return static_cast<typename Base::derived_t const&>(*this);
    }
  };

  template <class Base>
  class iterator_comparisons :
    public Base
  {
  };

#if BOOST_WORKAROUND(BOOST_MSVC, <=1200)
#define BOOST_ARG_DEP_TYPENAME
#else
#define BOOST_ARG_DEP_TYPENAME typename
#endif

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator==(iterator_comparisons<Base1> const& lhs,
             iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return iterator_core_access::equal(lhs.derived(),
                                       rhs.derived());
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator!=(iterator_comparisons<Base1> const& lhs,
             iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return !iterator_core_access::equal(lhs.derived(),
                                        rhs.derived());
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator<(iterator_comparisons<Base1> const& lhs,
                        iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) > 0;
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator>(iterator_comparisons<Base1> const& lhs,
                        iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) < 0;
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator<=(iterator_comparisons<Base1> const& lhs,
                        iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) >= 0;
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           bool>::type
  operator>=(iterator_comparisons<Base1> const& lhs,
             iterator_comparisons<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    return iterator_core_access::distance_to(lhs.derived(),
                                             rhs.derived()) <= 0;
  }


  template <class Base>
  class iterator_arith :
    public Base 
  {
  };

  template <class Base>
  inline
  typename Base::derived_t operator+(iterator_arith<Base> const&    i,
                                     typename Base::difference_type n)
  {
    typename Base::derived_t tmp(i.derived());
    return tmp += n;
  }

  template <class Base>
  inline
  typename Base::derived_t operator+(typename Base::difference_type n,
                                     iterator_arith<Base> const&    i)
  {
    typename Base::derived_t tmp(i.derived());
    return tmp += n;
  }

  template <class Base1,
            class Base2>
  inline
  typename detail::enable_if_interoperable<typename Base1::derived_t,
                                           typename Base2::derived_t,
                                           typename Base1::difference_type>::type
  operator-(iterator_arith<Base1> const& lhs,
            iterator_arith<Base2> const& rhs)
  {
    // For those compilers that do not support enable_if
    BOOST_STATIC_ASSERT((detail::is_interoperable< 
                         BOOST_ARG_DEP_TYPENAME Base1::derived_t,
                         BOOST_ARG_DEP_TYPENAME Base2::derived_t >::value));

    BOOST_STATIC_ASSERT((is_same<BOOST_ARG_DEP_TYPENAME Base1::difference_type,
                                 BOOST_ARG_DEP_TYPENAME Base2::difference_type>::value));

    return iterator_core_access::distance_to(rhs.derived(),
                                             lhs.derived());
  }

  template <class Derived,
            class V,
            class R,
            class P,
            class C, 
            class D,
            // We do not use the name base here, as base is used in
            // reverse iterator.
            class Super = iterator_arith<
    iterator_comparisons<
    downcastable<
    repository< Derived, V, R, P, C, D > > > >
  >
  class iterator_facade : 
    public Super
  {
    typedef Super super_t;

  public:
    typedef typename super_t::reference       reference;
    typedef typename super_t::difference_type difference_type;

    reference operator*() const
    { return iterator_core_access::dereference(this->derived()); }

    // Needs eventual help for input iterators
    P operator->() const { return &iterator_core_access::dereference(this->derived()); }
        
    reference operator[](difference_type n) const
    { return *(*this + n); }

    Derived& operator++()
    { iterator_core_access::increment(this->derived()); return this->derived(); }

    Derived operator++(int)
    { Derived tmp(this->derived()); ++*this; return tmp; }
  
    Derived& operator--()
    { iterator_core_access::decrement(this->derived()); return this->derived(); }
  
    Derived operator--(int)
    { Derived tmp(this->derived()); --*this; return tmp; }
  
    Derived& operator+=(difference_type n)
    { iterator_core_access::advance(this->derived(), n); return this->derived(); }
  
    Derived& operator-=(difference_type n)
    { iterator_core_access::advance(this->derived(), -n); return this->derived(); }
  
    Derived operator-(difference_type x) const
    { Derived result(this->derived()); return result -= x; }
  };

  //
  // TODO Handle default arguments the same way as
  // in former ia lib
  //
  template <class Derived,
            class Iterator,
            class Value     = typename detail::iterator_traits<Iterator>::value_type,
            class Reference = typename detail::iterator_traits<Iterator>::reference,
            class Pointer   = typename detail::iterator_traits<Iterator>::pointer,
            class Category  = typename detail::iterator_traits<Iterator>::iterator_category,
            class Distance  = typename detail::iterator_traits<Iterator>::difference_type>
  class iterator_adaptor :
    public iterator_facade<Derived,
                           Value,
                           Reference,
                           Pointer,
                           Category,
                           Distance>
  {
    friend class iterator_core_access;

  public:
    iterator_adaptor() {}

    explicit iterator_adaptor(Iterator iter)
      : m_iterator(iter)
    {
    }

    Iterator base() const { return m_iterator; }

  protected:
    // Core iterator interface for iterator_facade
    // 
    Reference dereference() const { return *m_iterator; }

    template <class OtherDerived,
              class OtherIterator,
              class OtherValue,     
              class OtherReference, 
              class OtherPointer >   
    bool equal(iterator_adaptor<OtherDerived,
                                OtherIterator,
                                OtherValue,
                                OtherReference,
                                OtherPointer,
                                Category,
                                Distance> const& x) const
    {
      return m_iterator == x.base();
    }
  
    void advance(Distance n)
    {
      m_iterator += n;
    }
  
    void increment() { ++m_iterator; }
    void decrement() { --m_iterator; }

    template <class OtherDerived,
              class OtherIterator,
              class OtherValue,     
              class OtherReference, 
              class OtherPointer >   
    Distance distance_to(iterator_adaptor<OtherDerived,
                                          OtherIterator,
                                          OtherValue,
                                          OtherReference,
                                          OtherPointer,
                                          Category,
                                          Distance> const& y) const
    {
      return y.base() - m_iterator;
    }

  private:
    Iterator m_iterator;

  };

  //
  //
  //
  template <class Iterator>
  class reverse_iterator :
    public iterator_adaptor< reverse_iterator<Iterator>, Iterator >
  {
    typedef iterator_adaptor< reverse_iterator<Iterator>, Iterator > super_t;

    friend class iterator_core_access;

  public:
    reverse_iterator() {}

    explicit reverse_iterator(Iterator x) 
      : super_t(x) {}

    template<class OtherIterator>
    reverse_iterator(reverse_iterator<OtherIterator> const& r,
                     typename enable_if_convertible<OtherIterator, Iterator>::type* = 0)
      : super_t(r.base()) {}

  private:
    typename super_t::reference dereference() const { return *boost::prior(this->base()); }
    
    void increment() { super_t::decrement(); }
    void decrement() { super_t::increment(); }

    void advance(typename super_t::difference_type n)
    {
      super_t::advance(-n);
    }

    template <class OtherIterator>
    typename super_t::difference_type
    distance_to(reverse_iterator<OtherIterator> const& y) const
    {
      return -super_t::distance_to(y);
    }
  };

  //
  // TODO fix category
  //
  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator :
    public iterator_adaptor< transform_iterator<AdaptableUnaryFunction, Iterator>,
                             Iterator,
                             typename AdaptableUnaryFunction::result_type,
                             typename AdaptableUnaryFunction::result_type, 
                             typename AdaptableUnaryFunction::result_type*
                             >
  {
    typedef iterator_adaptor< transform_iterator<AdaptableUnaryFunction, Iterator>,
                              Iterator,
                              typename AdaptableUnaryFunction::result_type,
                              typename AdaptableUnaryFunction::result_type, 
                              typename AdaptableUnaryFunction::result_type* > super_t;

    friend class iterator_core_access;

  public:
    transform_iterator() { }

    transform_iterator(Iterator const&        x,
                       AdaptableUnaryFunction f)
      : super_t(x), m_f(f) { }

    template<class OtherIterator>
    transform_iterator(transform_iterator<AdaptableUnaryFunction, OtherIterator> const& t,
                       typename enable_if_convertible<OtherIterator, Iterator>::type* = 0)
      : super_t(t.base()), m_f(t.functor()) {}

    AdaptableUnaryFunction functor() const { return m_f; }

  private:
    typename super_t::value_type dereference() const { return m_f(super_t::dereference()); }

    AdaptableUnaryFunction m_f;
  };

  struct unspecified {};

  namespace detail
  {
    //
    // Detection for whether a type has a nested `element_type'
    // typedef. Used to detect smart pointers. We're having trouble
    // auto-detecting smart pointers with gcc-2.95 via the nested
    // element_type member. However, we really ought to have a
    // specializable is_pointer template which can be used instead with
    // something like boost/python/pointee.hpp to find the value_type.
    //
    namespace aux
    {
      BOOST_MPL_HAS_XXX_TRAIT_DEF(element_type)
        }

    template <class T>
    struct has_element_type
      : mpl::if_<
      is_class<T>
// gcc 2.95 doesn't seem to be able to detect element_type without barfing    
# if BOOST_WORKAROUND(__GNUC__, == 2 && __GNUC_MINOR__ == 95)
      , mpl::bool_c<false>
# else 
      , aux::has_element_type<T>
# endif 
      , mpl::bool_c<false>
    >::type
    {
    };
  
    // Metafunction returning the nested element_type typedef
    template <class T>
    struct smart_pointer_traits
    {
      typedef typename remove_const<
        typename T::element_type
      >::type value_type;

      typedef typename T::element_type& reference;
      typedef typename T::element_type* pointer;
    };

    // If the Value parameter is unspecified, we use this metafunction
    // to deduce the default types
    template <class Iter>
    struct indirect_defaults
      : mpl::if_c<
      has_element_type<typename iterator_traits<Iter>::value_type>::value
      , smart_pointer_traits<typename iterator_traits<Iter>::value_type>
      , iterator_traits<typename iterator_traits<Iter>::value_type>
    >::type
    {
      typedef typename iterator_traits<Iter>::iterator_category iterator_category;
      typedef typename iterator_traits<Iter>::difference_type difference_type;
    };

    template <class Base, class Traits>
    struct indirect_traits
      : mpl::if_<is_same<Traits,unspecified>, indirect_defaults<Base>, Traits>::type
    {
    };
  } // namespace detail

  template <class Iterator, class Traits = unspecified>
  class indirect_iterator :
    public iterator_adaptor< indirect_iterator<Iterator, Traits>,
                             Iterator,
                             typename detail::indirect_traits<Iterator, Traits>::value_type,
                             typename detail::indirect_traits<Iterator, Traits>::reference,
                             typename detail::indirect_traits<Iterator, Traits>::pointer,
                             typename detail::indirect_traits<Iterator, Traits>::iterator_category,
                             typename detail::indirect_traits<Iterator, Traits>::difference_type >
  {
    typedef iterator_adaptor< indirect_iterator<Iterator, Traits>,
                              Iterator,
                              typename detail::indirect_traits<Iterator, Traits>::value_type,
                              typename detail::indirect_traits<Iterator, Traits>::reference,
                              typename detail::indirect_traits<Iterator, Traits>::pointer,
                              typename detail::indirect_traits<Iterator, Traits>::iterator_category,
                              typename detail::indirect_traits<Iterator, Traits>::difference_type > super_t;

    friend class iterator_core_access;

  public:
    indirect_iterator() {}

    indirect_iterator(Iterator iter)
      : super_t(iter) {}

    template <class OtherIterator,
              class OtherTraits>
    indirect_iterator(indirect_iterator<OtherIterator, OtherTraits> const& y,
                      typename enable_if_convertible<OtherIterator, Iterator>::type* = 0)
      : super_t(y.base())  {}

  private:    
    typename super_t::reference dereference() const { return **this->base(); }

  };

  template <class Iter>
  inline
  indirect_iterator<Iter> make_indirect_iterator(Iter x)
  {
    return indirect_iterator<Iter>(x);
  }

  template <class Traits, class Iter>
  inline
  indirect_iterator<Iter,Traits> make_indirect_iterator(Iter x, Traits* = 0)
  {
    return indirect_iterator<Iter, Traits>(x);
  }

} // namespace boost


#endif // BOOST_ITERATOR_ADAPTORS_HPP
