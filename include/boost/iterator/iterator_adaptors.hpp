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
#include <boost/mpl/logical/and.hpp>
#include <boost/mpl/identity.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include "boost/type_traits/detail/bool_trait_def.hpp"

#if BOOST_WORKAROUND(BOOST_MSVC,  <= 1300)                      \
 || BOOST_WORKAROUND(__GNUC__, <= 2 && __GNUC_MINOR__ <= 95)    \
 || BOOST_WORKAROUND(__MWERKS__, <= 0x3000)
#  define BOOST_NO_SFINAE // "Substitution Failure Is Not An Error not implemented"
#endif

#if BOOST_WORKAROUND(BOOST_MSVC, <=1200)
#  define BOOST_ARG_DEP_TYPENAME
#else
#  define BOOST_ARG_DEP_TYPENAME typename
#endif

#if BOOST_WORKAROUND(__MWERKS__, <=0x2407)
#  define BOOST_NO_IS_CONVERTIBLE // "is_convertible doesn't always work"
#endif

#if BOOST_WORKAROUND(__GNUC__, == 2 && __GNUC_MINOR__ == 95)    \
  || BOOST_WORKAROUND(__MWERKS__, <= 0x2407)                    \
  || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
# define BOOST_NO_MPL_AUX_HAS_XXX  // "MPL's has_xxx facility doesn't work"
#endif 

#ifdef BOOST_NO_MPL_AUX_HAS_XXX
# include <boost/shared_ptr.hpp>
# include <boost/scoped_ptr.hpp>
# include <memory>
#endif 


namespace boost
{

  namespace detail
  {
    //
    // Base machinery for all kinds of enable if
    //
    template<bool>
    struct enabled
    {
      template<typename T>
      struct base
      {
        typedef T type;
      };
    };
    
    //
    // For compilers that don't support "Substitution Failure Is Not An Error"
    // enable_if falls back to always enabled. See comments
    // on operator implementation for consequences.
    //
    template<>
    struct enabled<false>
    {
      template<typename T>
      struct base
      {
#ifdef BOOST_NO_SFINAE

        typedef T type;

        // This way to do it would give a nice error messages containing
        // invalid overload, but has the big disadvantage that
        // there is no reference to user code in the error message.
        //
        // struct invalid_overload;
        // typedef invalid_overload type;
        //
#endif
      };
    };

    //
    // Meta function that determines whether two
    // iterator types are considered interoperable.
    //
    // Two iterator types A,B are considered interoperable if either
    // A is convertible to B or vice versa.
    // This interoperability definition is in sync with the
    // standards requirements on constant/mutable container
    // iterators (23.1 [lib.container.requirements]).
    //
    // For compilers that don't support is_convertible 
    // is_interoperable gives false positives. See comments
    // on operator implementation for consequences.
    //
    template <typename A, typename B>
    struct is_interoperable
#if defined(BOOST_NO_IS_CONVERTIBLE)
      : mpl::true_c
#else
      : mpl::logical_or<
           is_convertible< A, B >
         , is_convertible< B, A > >
#endif
    {
    };

    //
    // enable if for use in operator implementation.
    //
    // enable_if_interoperable falls back to always enabled for compilers
    // that don't support enable_if or is_convertible. 
    //
    template <class Facade1,
              class Facade2,
              class Return>
    struct enable_if_interoperable
# if !defined(BOOST_NO_SFINAE) && !defined(BOOST_NO_IS_CONVERTIBLE)
      : detail::enabled<
           ::boost::detail::is_interoperable<Facade1, Facade2>::value
        >::template base<Return>
# else
      : mpl::identity<Return>
# endif 
    {
# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
        typedef Return type;
# endif 
    };

    // 
    // Result type used in enable_if_convertible meta function.
    // This can be an incomplete type, as only pointers to 
    // enable_if_convertible< ... >::type are used.
    // We could have used void for this, but conversion to
    // void* is just to easy.
    //
    struct enable_type;

    // traits_iterator<It> has two important properties:
    //
    //   1. It is derived from boost::iterator<...>, which is
    //      important for standard library interoperability of
    //      iterator types on some (broken) implementations.
    //
    //   2. The associated types are taken from iterator_traits<It>.
    //
    // It might arguably be better to arrange for
    // boost::detail::iterator_traits<It> to be derived from
    // boost::iterator<...>, then we could use
    // boost::detail::iterator_traits directly.
    template <class Iterator>
    struct traits_iterator
         : iterator<
             typename iterator_traits<Iterator>::iterator_category
           , typename iterator_traits<Iterator>::value_type
           , typename iterator_traits<Iterator>::difference_type
           , typename iterator_traits<Iterator>::pointer
           , typename iterator_traits<Iterator>::reference
        >
    {
    };

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
  //   adapted_iterator(OtherIterator const& it,
  //                    typename enable_if_convertible<OtherIterator, Iterator>::type* = 0);
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
  template<
      typename From
    , typename To>
  struct enable_if_convertible
  {
      // Borland 551 and vc6 have a problem with the use of base class
      // forwarding in this template, so  we write it all out here
# if defined(BOOST_NO_IS_CONVERTIBLE) || defined(BOOST_NO_SFINAE)
      typedef detail::enable_type type;
# else
      typedef typename detail::enabled<
#  if BOOST_WORKAROUND(_MSC_FULL_VER, BOOST_TESTED_AT(13102292) && BOOST_MSVC > 1300)
          // For some reason vc7.1 needs us to "cut off" instantiation
          // of is_convertible in the case where From == To.
          mpl::logical_or<is_same<From,To>, is_convertible<From, To> >::value
#  else 
          ::boost::is_convertible<From, To>::value
#  endif 
      >::template base<detail::enable_type>::type type;
# endif 
  };

  //
  // Helper class for granting access to the iterator core interface.
  //
  // The simple core interface is used by iterator_facade. The core
  // interface of a user/library defined iterator type should not be made public
  // so that it does not clutter the public interface. Instead iterator_core_access
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

  namespace detail
  {
    struct empty_base {};
  }
  
  // Encapsulates the "Curiously Recursive Template" pattern.
  // Derived should be a class derived from this instantiation, and
  // Base will be inserted as a base class.
  template <class Derived, class Base = detail::empty_base>
  class downcastable
      : public Base
  {
   public:
      typedef Derived derived_t;
      
      Derived& derived()
      {
          return static_cast<Derived&>(*this);
      }

      Derived const& derived() const 
      {
          return static_cast<Derived const&>(*this);
      }
  };

  template <class Base>
  class iterator_comparisons
      : public Base
  {
  };

  //
  // Operator implementation. The library supplied operators 
  // enables the user to provide fully interoperable constant/mutable
  // iterator types. I.e. the library provides all operators
  // for all mutable/constant iterator combinations.
  //
  // Note though that this kind of interoperability for constant/mutable
  // iterators is not required by the standard for container iterators.
  // All the standard asks for is a conversion mutable -> constant.
  // Most standard library implementations nowadays provide fully interoperable
  // iterator implementations, but there are still heavily used implementations 
  // that do not provide them. (Actually it's even worse, they do not provide 
  // them for only a few iterators.)
  //
  // ?? Maybe a BOOST_ITERATOR_NO_FULL_INTEROPERABILITY macro should
  //    enable the user to turn off mixed type operators 
  //
  // The library takes care to provide only the right operator overloads.
  // I.e.
  //
  // bool operator==(Iterator,      Iterator);
  // bool operator==(ConstIterator, Iterator);
  // bool operator==(Iterator,      ConstIterator);
  // bool operator==(ConstIterator, ConstIterator);
  //
  //   ...
  //
  // In order to do so it uses c++ idioms that are not yet widely supported
  // by current compiler releases. The library is designed to degrade gracefully
  // in the face of compiler deficiencies. In general compiler
  // deficiencies result in less strict error checking and more obscure
  // error messages, functionality is not affected.
  //
  // For full operation compiler support for "Substitution Failure Is Not An Error" 
  // (aka. enable_if) and boost::is_convertible is required.
  //
  // The following problems occur if support is lacking.
  //
  // Pseudo code
  //
  // ---------------
  // AdaptorA<Iterator1> a1;
  // AdaptorA<Iterator2> a2;
  //
  // // This will result in a no such overload error in full operation
  // // If enable_if or is_convertible is not supported
  // // The instantiation will fail with an error hopefully indicating that
  // // there is no operator== for Iterator1, Iterator2
  // // The same will happen if no enable_if is used to remove
  // // false overloads from the templated conversion constructor
  // // of AdaptorA.
  //
  // a1 == a2; 
  // ----------------
  //
  // AdaptorA<Iterator> a;
  // AdaptorB<Iterator> b;
  //
  // // This will result in a no such overload error in full operation
  // // If enable_if is not supported the static assert used
  // // in the operator implementation will fail.
  // // This will accidently work if is_convertible is not supported.
  //
  // a == b;
  // ----------------
  //
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
  typename detail::enable_if_interoperable<
      typename Base1::derived_t
      , typename Base2::derived_t
      , typename Base1::difference_type
      >::type
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

  template <
        class Derived
      , class Traits
      , class Super = iterator_arith<  
                        iterator_comparisons<
                            downcastable<Derived, Traits> > >
  >
  class iterator_facade
      : public Super
  {
      typedef Super super_t;
   public:
    typedef typename super_t::reference       reference;
    typedef typename super_t::difference_type difference_type;
    typedef typename super_t::pointer pointer;

    reference operator*() const
    { return iterator_core_access::dereference(this->derived()); }

    // Needs eventual help for input iterators
    pointer operator->() const { return &iterator_core_access::dereference(this->derived()); }
        
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

  namespace detail
  {
      template <class Traits, class Other>
      struct same_category_and_difference
          : mpl::logical_and<
              is_same<
                 typename Traits::iterator_category
               , typename Other::iterator_category
              >
            , is_same<
                 typename Traits::iterator_category
               , typename Other::iterator_category
              >
            >
      {};
  }
  
  //
  // TODO Handle default arguments the same way as
  // in former ia lib
  //
  template <
      class Derived
      , class Iterator
      , class Traits = detail::traits_iterator<Iterator>
  >
  class iterator_adaptor
      : public iterator_facade<Derived,Traits>
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
    typename Traits::reference dereference() const { return *m_iterator; }

    template <
        class OtherDerived, class OtherIterator, class OtherTraits
    >   
    bool equal(iterator_adaptor<OtherDerived,OtherIterator,OtherTraits> const& x) const
    {
        BOOST_STATIC_ASSERT(
            (detail::same_category_and_difference<Traits,OtherTraits>::value)
            );
        return m_iterator == x.base();
    }
  
    void advance(typename Traits::difference_type n)
    {
        m_iterator += n;
    }
  
    void increment() { ++m_iterator; }
    void decrement() { --m_iterator; }

    template <class OtherDerived, class OtherIterator, class OtherTraits>   
    typename Traits::difference_type distance_to(
        iterator_adaptor<OtherDerived, OtherIterator, OtherTraits> const& y) const
    {
        BOOST_STATIC_ASSERT(
            (detail::same_category_and_difference<Traits,OtherTraits>::value)
            );
        return y.base() - m_iterator;
    }

   private: // data members
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
    reverse_iterator(
        reverse_iterator<OtherIterator> const& r
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
        )
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

  template <class BidirectionalIterator>
  reverse_iterator<BidirectionalIterator> make_reverse_iterator(BidirectionalIterator x)
  {
      return reverse_iterator<BidirectionalIterator>(x);
  }

  // Given the transform iterator's transformation and iterator, this
  // is the type used as its traits.
  template <class AdaptableUnaryFunction, class Iterator>
  struct transform_iterator_traits
      : iterator<
              typename detail::iterator_traits<Iterator>::iterator_category
            , typename AdaptableUnaryFunction::result_type
            , typename detail::iterator_traits<Iterator>::difference_type
            , typename AdaptableUnaryFunction::result_type*
            , typename AdaptableUnaryFunction::result_type
      >
  {};
   
  //
  // TODO fix category
  //
  template <class AdaptableUnaryFunction, class Iterator>
  class transform_iterator
    : public iterator_adaptor<
          transform_iterator<AdaptableUnaryFunction, Iterator>
        , Iterator
        , transform_iterator_traits<AdaptableUnaryFunction,Iterator>
      >
  {
    typedef iterator_adaptor<
        transform_iterator<AdaptableUnaryFunction, Iterator>
        , Iterator
        , transform_iterator_traits<AdaptableUnaryFunction,Iterator>
    > super_t;

    friend class iterator_core_access;

  public:
    transform_iterator() { }

    transform_iterator(Iterator const&        x,
                       AdaptableUnaryFunction f)
      : super_t(x), m_f(f) { }

    template<class OtherIterator>
    transform_iterator(
        transform_iterator<AdaptableUnaryFunction, OtherIterator> const& t
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
        )
      : super_t(t.base()), m_f(t.functor()) {}

    AdaptableUnaryFunction functor() const { return m_f; }

  private:
    typename super_t::value_type dereference() const { return m_f(super_t::dereference()); }

    // Probably should be the initial base class so it can be
    // optimized away via EBO if it is an empty class.
    AdaptableUnaryFunction m_f;
  };

  struct unspecified {};

  namespace detail
  {
    //
    // Detection for whether a type has a nested `element_type'
    // typedef. Used to detect smart pointers. For compilers not
    // supporting mpl's has_xxx, we supply specializations. However, we
    // really ought to have a specializable is_pointer template which
    // can be used instead with something like
    // boost/python/pointee.hpp to find the value_type.
    //
# if !defined BOOST_NO_MPL_AUX_HAS_XXX
    namespace aux
    {
      BOOST_MPL_HAS_XXX_TRAIT_DEF(element_type)
    }

    template <class T>
    struct has_element_type
      : mpl::if_<
          is_class<T>
        , aux::has_element_type<T>
        , mpl::false_c
      >::type
    {
    };
# else
    template <class T>
    struct has_element_type
        : mpl::false_c {};
    
    template <class T>
    struct has_element_type<boost::shared_ptr<T> >
        : mpl::true_c {};
    
    template <class T>
    struct has_element_type<boost::scoped_ptr<T> >
        : mpl::true_c {};
    
    template <class T>
    struct has_element_type<std::auto_ptr<T> >
        : mpl::true_c {};
# endif 
  
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
      : mpl::if_<
            has_element_type<typename iterator_traits<Iter>::value_type>
          , smart_pointer_traits<typename iterator_traits<Iter>::value_type>
          , iterator_traits<typename iterator_traits<Iter>::value_type>
        >::type
    {
      typedef typename iterator_traits<Iter>::iterator_category iterator_category;
      typedef typename iterator_traits<Iter>::difference_type difference_type;
    };

    // The traits to use for indirect iterator, by default.  Whatever
    // is supplied gets passed through traits_iterator<...> so that it
    // is ultimately derived from boost::iterator<...>
    template <class Base, class Traits>
    struct indirect_traits
      : traits_iterator<
           typename mpl::if_<
              is_same<Traits,unspecified>
            , indirect_defaults<Base>
            , Traits
           >::type
        >
    {
    };
  } // namespace detail

  template <class Iterator, class Traits = unspecified>
  class indirect_iterator :
    public iterator_adaptor<
        indirect_iterator<Iterator, Traits>
      , Iterator
      , detail::indirect_traits<Iterator,Traits> >
  {
    typedef iterator_adaptor<
          indirect_iterator<Iterator, Traits>
        , Iterator
        , detail::indirect_traits<Iterator,Traits>
    > super_t;

    friend class iterator_core_access;

  public:
    indirect_iterator() {}

    indirect_iterator(Iterator iter)
      : super_t(iter) {}

    template <class OtherIterator,
              class OtherTraits>
    indirect_iterator(
        indirect_iterator<OtherIterator, OtherTraits> const& y
        , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
        )
      : super_t(y.base())
      {}
      
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

  template <class Iterator>
  struct filter_iterator_traits
      : detail::iterator_traits<Iterator>
  {
      typedef iterator_tag<
            typename return_category<Iterator>::type
          , forward_traversal_tag
      > iterator_category;
  };

  template <class Predicate, class Iterator>
  class filter_iterator
      : public iterator_adaptor<
           filter_iterator<Predicate, Iterator>, Iterator
           , detail::traits_iterator<filter_iterator_traits<Iterator> >
        >
  {
      typedef iterator_adaptor<
           filter_iterator<Predicate, Iterator>, Iterator
          , detail::traits_iterator<filter_iterator_traits<Iterator> >
      > super_t;

      friend class iterator_core_access;

   public:
      filter_iterator() { }

      filter_iterator(Predicate f, Iterator x, Iterator end = Iterator())
          : super_t(x), m_predicate(f), m_end(end)
      {
          satisfy_predicate();
      }

      filter_iterator(Iterator x, Iterator end = Iterator())
          : super_t(x), m_predicate(), m_end(end)
      {
          satisfy_predicate();
      }

      template<class OtherIterator>
      filter_iterator(
          filter_iterator<Predicate, OtherIterator> const& t
          , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
          )
          : super_t(t.base()), m_predicate(t.predicate()), m_end(t.end()) {}

      Predicate predicate() const { return m_predicate; }

      Iterator end() const { return m_end; }

   private:
      void increment()
      {
          super_t::increment();
          satisfy_predicate();
      }

      void satisfy_predicate()
      {
          while (this->base() != this->m_end && !this->m_predicate(*this->base()))
              super_t::increment();
      }

      // Probably should be the initial base class so it can be
      // optimized away via EBO if it is an empty class.
      Predicate m_predicate;
      Iterator m_end;
  };

  template <class Predicate, class Iterator>
  filter_iterator<Predicate,Iterator>
  make_filter_iterator(Predicate f, Iterator x, Iterator end = Iterator())
  {
      return filter_iterator<Predicate,Iterator>(f,x,end);
  }
} // namespace boost

//
// clean up local workaround macros
//

# undef BOOST_NO_SFINAE
# undef BOOST_ARG_DEP_TYPENAME
# undef BOOST_NO_IS_CONVERTIBLE
# undef BOOST_NO_MPL_AUX_HAS_XXX

#endif // BOOST_ITERATOR_ADAPTORS_HPP
