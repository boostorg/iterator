// (C) Copyright Jeremy Siek 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

// TODO:
//   Add separate category tag for operator[].

#ifndef BOOST_ITERATOR_CATEGORIES_HPP
#define BOOST_ITERATOR_CATEGORIES_HPP

#include <boost/config.hpp>
#include <boost/iterator/detail/categories.hpp>

#include <boost/type_traits/conversion_traits.hpp>
#include <boost/type_traits/cv_traits.hpp>

#include <boost/python/detail/indirect_traits.hpp>

#include <boost/detail/iterator.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/mpl/apply_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/aux_/has_xxx.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/apply.hpp>

#include <iterator>

#if BOOST_WORKAROUND(__MWERKS__, <=0x2407)
#  define BOOST_NO_IS_CONVERTIBLE // "Convertible does not provide enough/is not working"
#endif

namespace boost {

  namespace detail
  {
    // Helper metafunction for std_category below
    template <class Cat, class Tag, class Next>
    struct match_tag
      : mpl::apply_if<is_tag<Tag, Cat>, mpl::identity<Tag>, Next>
    {
    };

    // Converts a possibly user-defined category tag to the
    // most-derived standard tag which is a base of that tag.
    template <class Category>
    struct std_category
      : match_tag<
            Category, std::random_access_iterator_tag
          , match_tag<Category, std::bidirectional_iterator_tag
              , match_tag<Category, std::forward_iterator_tag
                  , match_tag<Category, std::input_iterator_tag
                      , match_tag<Category, std::output_iterator_tag
#  if BOOST_WORKAROUND(BOOST_MSVC, <= 1300)
                          , mpl::identity<void>
#  else
                          , void
#  endif 
                        >
                    >
                >
            >
        >
    {
    };

    // std_to_new_tags --
    //
    //  A metafunction which converts any standard tag into its
    //  corresponding new-style traversal tag.
    //
    //  Also, instantiations are metafunction classes which convert a
    //  reference type into a corresponding new-style access tag.
    template <class Category> struct std_to_new_tags
# if BOOST_WORKAROUND(BOOST_MSVC, == 1300) // handle ETI
    {
        typedef void type;
        template <class T> struct apply { typedef void type; };
    }
# endif
    ;

# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200) // handle ETI
    template <> struct std_to_new_tags<int> {};
# endif

    //
    // Specializations for specific standard tags
    //
    template <>
    struct std_to_new_tags<std::input_iterator_tag>
    {
        typedef single_pass_iterator_tag type;
        
        template <class Reference>
        struct apply
          : mpl::identity<readable_iterator_tag> {};

    };
    
    template <>
    struct std_to_new_tags<std::output_iterator_tag>
    {
        typedef incrementable_iterator_tag type;
        
        template <class Reference>
        struct apply
          : mpl::identity<writable_iterator_tag> {};
    };
    
    template <>
    struct std_to_new_tags<std::forward_iterator_tag>
    {
        typedef forward_traversal_tag type;
        
        template <class Reference>
        struct apply
          : mpl::if_<
                python::detail::is_reference_to_const<Reference>
              , boost::readable_lvalue_iterator_tag
              , boost::writable_lvalue_iterator_tag
        >
        {};
    };
    
    template <>
    struct std_to_new_tags<std::bidirectional_iterator_tag>
      : std_to_new_tags<std::forward_iterator_tag>
    {
        typedef bidirectional_traversal_tag type;
    };

    template <>
    struct std_to_new_tags<std::random_access_iterator_tag>
      : std_to_new_tags<std::bidirectional_iterator_tag>
    {
        typedef random_access_traversal_tag type;
    };

    template <class Category>
    struct old_tag_converter
      : std_to_new_tags<
            typename std_category<Category>::type
        >
    {
    };
    
    template <typename Category>
    struct iter_category_to_traversal
      : std_to_new_tags<
            typename std_category<Category>::type
        >
    {};

    template <typename Category, typename Reference>
    struct iter_category_to_access
      : mpl::apply1<
            iter_category_to_traversal<Category>
          , Reference
        >
    {};

# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
    // Deal with ETI
    template <> struct iter_category_to_access<int, int> {};
    template <> struct iter_category_to_traversal<int> {};
# endif

    // A metafunction returning true iff T is boost::iterator_tag<R,U>
    template <class T>
    struct is_boost_iterator_tag;
    
#if BOOST_WORKAROUND(__MWERKS__, <= 0x2407)
    //
    // has_xxx fails, so we have to use 
    // something less sophisticated.
    // 
    // The solution depends on the fact that only
    // std iterator categories work with is_xxx_iterator
    // meta functions, as BOOST_NO_IS_CONVERTIBLE is
    // defined for cwpro7.
    //
    template <class Tag>
    struct is_new_iterator_tag
      : mpl::not_<
            mpl::or_<
                is_tag<std::input_iterator_tag, Tag>
              , is_tag<std::output_iterator_tag, Tag>
            >
        >
    {};

#elif BOOST_WORKAROUND(__GNUC__, == 2 && __GNUC_MINOR__ == 95) \
   || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
    
    template <class Tag>
    struct is_new_iterator_tag
        : is_boost_iterator_tag<Tag>
    {
    };
    
#else
    
  BOOST_MPL_HAS_XXX_TRAIT_DEF(traversal)

    template <class Tag>
    struct is_new_iterator_tag
      : mpl::if_<
            is_class<Tag>
          , has_traversal<Tag>
          , mpl::false_
        >::type
    {
    };

#endif

  } // namespace detail
  
  namespace detail {

    template <class NewCategoryTag>
    struct get_access_category {
      typedef typename NewCategoryTag::returns type;
    };
    template <class NewCategoryTag>
    struct get_traversal_category {
      typedef typename NewCategoryTag::traversal type;
    };

    template <class CategoryTag, class Reference>
    struct access_category_tag
        : mpl::apply_if< 
             is_new_iterator_tag<CategoryTag>
           , get_access_category<CategoryTag>
           , iter_category_to_access<CategoryTag, Reference>
    >
    {
    };
  
    template <class CategoryTag>
    struct traversal_category_tag
      : mpl::apply_if< 
            is_new_iterator_tag<CategoryTag>
          , get_traversal_category<CategoryTag>
          , iter_category_to_traversal<CategoryTag>
        >
    {
    };

# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200)
    // Deal with ETI
    template <> struct access_category_tag<int, int> {};
    template <> struct traversal_category_tag<int> {};
# endif
  
  } // namespace detail

  template <class Iterator>
  struct access_category
      : detail::access_category_tag<
           typename detail::iterator_traits<Iterator>::iterator_category
          , typename detail::iterator_traits<Iterator>::reference>
  {};

  template <class Iterator>
  struct traversal_category
    : detail::traversal_category_tag<
          typename detail::iterator_traits<Iterator>::iterator_category
      >
  {
  };

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  template <typename T>
  struct access_category<T*>
      : mpl::if_<
          is_const<T>
          , readable_lvalue_iterator_tag
          , writable_lvalue_iterator_tag>
  {
  };

  template <typename T>
  struct traversal_category<T*>
  {
    typedef random_access_traversal_tag type;
  };

#endif

  template <class ReturnTag, class TraversalTag>
  struct iterator_tag
    : detail::minimum_category<
          typename ReturnTag::max_category
        , typename TraversalTag::max_category
      >
  {
    typedef ReturnTag returns;
    typedef TraversalTag traversal;
  };

  namespace detail
  {
# ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    template <class T>
    struct is_boost_iterator_tag
        : mpl::false_ {};

    template <class R, class T>
    struct is_boost_iterator_tag<iterator_tag<R,T> >
        : mpl::true_ {};
# else
    template <class T>
    struct is_boost_iterator_tag
    {
        typedef char (&yes)[1];
        typedef char (&no)[2];
        
        template <class R, class U>
        static yes test(mpl::identity<iterator_tag<R,U> >*);
        static no test(...);
    
        static mpl::identity<T>* inst;
        BOOST_STATIC_CONSTANT(bool, value = sizeof(test(inst)) == sizeof(yes));
        typedef mpl::bool_<value> type;
    };
# endif
  }

} // namespace boost

#undef BOOST_NO_IS_CONVERTIBLE

#endif // BOOST_ITERATOR_CATEGORIES_HPP
