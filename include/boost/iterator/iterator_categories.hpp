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
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/type_traits/cv_traits.hpp>
#include <boost/detail/iterator.hpp>
#include <boost/mpl/apply_if.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool_c.hpp>
#include <boost/mpl/aux_/has_xxx.hpp>
#include <boost/mpl/logical/or.hpp>
#include <boost/mpl/logical/and.hpp>
#include <iterator>

namespace boost {

  // Return Type Categories
  struct readable_iterator_tag { };
  struct writable_iterator_tag { };
  struct swappable_iterator_tag : 
    virtual public readable_iterator_tag, // Not sure about this change -JGS
    virtual public writable_iterator_tag { };
  struct constant_lvalue_iterator_tag : 
    virtual public readable_iterator_tag { };
  struct mutable_lvalue_iterator_tag : 
    virtual public swappable_iterator_tag,
    virtual public constant_lvalue_iterator_tag { };

  // Traversal Categories
  struct input_traversal_tag { };
  struct output_traversal_tag { };
  struct forward_traversal_tag : virtual public input_traversal_tag, 
    virtual public output_traversal_tag { };
  struct bidirectional_traversal_tag : virtual public forward_traversal_tag { };
  struct random_access_traversal_tag : virtual public bidirectional_traversal_tag { };

  struct error_iterator_tag { };

  // When creating new iterators, you have three options. 

  // 1) The recommended option is to specialize the return_category
  // and traversal_category for your iterator. However, if your
  // iterator is a templates, this will cause a problem on compilers
  // that do not have partial specialization.
  // 2) The second option is to put two nested typedefs in your
  // iterator, return_category and traversal_category, and derive your
  // iterator from new_iterator_base. 
  // 3) The third option is the create your iterator in the old STL
  // style and let the backward compatibility parts of this header
  // handle the rest. 

  // BTW, if you choose one of the first two options, you can still
  // implement all the old-style iterator typedefs, they will not get
  // in the way of the new categories.

  // Inherit from iterator_base if your iterator defines its own
  // return_category and traversal_category. Otherwise, the "old style"
  // iterator category will be mapped to the return_category and
  // traversal_category.
  struct new_iterator_base { };

  namespace detail
  {

    template <typename ValueType>
    struct choose_lvalue_return
        : mpl::if_<
            is_const<ValueType>
            , boost::constant_lvalue_iterator_tag
            , boost::mutable_lvalue_iterator_tag>
    {
    };
    
    
    template <typename Category, typename ValueType>
    struct iter_category_to_return
        : mpl::if_<
           is_convertible<Category*, std::forward_iterator_tag*>
           , typename choose_lvalue_return<ValueType>::type
           , typename mpl::if_<
                is_convertible<Category*, std::input_iterator_tag*>
                , boost::readable_iterator_tag
                , typename mpl::if_<
                    is_convertible<Category*, std::output_iterator_tag*>
                    , boost::writable_iterator_tag
                    , boost::error_iterator_tag
                  >::type
             >::type
         >
    {
    };

    template <typename Category>
    struct iter_category_to_traversal
      : mpl::if_<
          is_convertible<Category*, std::random_access_iterator_tag*>
          , random_access_traversal_tag
          , typename mpl::if_<
              is_convertible<Category*, std::bidirectional_iterator_tag*>
              , bidirectional_traversal_tag
              , typename mpl::if_<
                  is_convertible<Category*, std::forward_iterator_tag*>
                  , forward_traversal_tag
                  , typename mpl::if_<
                      is_convertible<Category*, std::input_iterator_tag*>
                      , input_traversal_tag
                      , output_traversal_tag
                    >::type
                >::type
            >::type
        >
    {
    };

  BOOST_MPL_HAS_XXX_TRAIT_DEF(traversal)

    template <class Tag>
    struct is_new_iterator_tag
        : //has_traversal<Tag>
          mpl::if_<
          is_class<Tag>
          , has_traversal<Tag>
          , mpl::bool_c<false> >::type
    {
    };
  } // namespace detail
  
  namespace detail {

    template <class NewCategoryTag>
    struct get_return_category {
      typedef typename NewCategoryTag::returns type;
    };
    template <class NewCategoryTag>
    struct get_traversal_category {
      typedef typename NewCategoryTag::traversal type;
    };

    template <class CategoryTag, class Value>
    struct return_category_tag
        : mpl::apply_if< 
           is_new_iterator_tag<CategoryTag>
           , get_return_category<CategoryTag>
           , iter_category_to_return<CategoryTag, Value>
    >
    {
    };
  
    template <class CategoryTag, class Value>
    struct traversal_category_tag
        : mpl::apply_if< 
	  is_new_iterator_tag<CategoryTag>
	  , get_traversal_category<CategoryTag>
	  , iter_category_to_traversal<CategoryTag>
	>
    {
    };

  } // namespace detail

  template <class Iterator>
  struct return_category
      : detail::return_category_tag<
           typename detail::iterator_traits<Iterator>::iterator_category
          , typename detail::iterator_traits<Iterator>::value_type>
  {};

  template <class Iterator>
  struct traversal_category
      : detail::traversal_category_tag<
          typename detail::iterator_traits<Iterator>::iterator_category
         , typename detail::iterator_traits<Iterator>::value_type>
  {
  };

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  template <typename T>
  struct return_category<T*>
      : mpl::if_<
          is_const<T>
          , constant_lvalue_iterator_tag
          , mutable_lvalue_iterator_tag>
  {
  };

  template <typename T>
  struct traversal_category<T*>
  {
    typedef random_access_traversal_tag type;
  };

#endif

  template <class RC, class TC>
  struct cvt_iterator_category
      : mpl::if_<
          mpl::logical_or<
             is_convertible<RC*, mutable_lvalue_iterator_tag*>
             , is_convertible<RC*, constant_lvalue_iterator_tag*>
          >
          , typename mpl::if_<
              is_convertible<TC*, random_access_traversal_tag*>
              , std::random_access_iterator_tag
              , typename mpl::if_<
                  is_convertible<TC*, bidirectional_traversal_tag*>
                  , std::bidirectional_iterator_tag
                  , typename mpl::if_<
                      is_convertible<TC*, forward_traversal_tag*>
                      , std::forward_iterator_tag
                      , error_iterator_tag
                    >::type
                  >::type
                >::type

          , typename mpl::if_<
            mpl::logical_and<
               is_convertible<RC*, readable_iterator_tag*>
               , is_convertible<TC*, input_traversal_tag*>
            >
            , std::input_iterator_tag
            , typename mpl::if_<
                mpl::logical_and<
                  is_convertible<RC*, writable_iterator_tag*>
                  , is_convertible<TC*, output_traversal_tag*>
                >
                , std::output_iterator_tag
                , error_iterator_tag
             >::type
          >::type 
    >
  {
  };

  template <class ReturnTag, class TraversalTag>
  struct iterator_tag : cvt_iterator_category<ReturnTag, TraversalTag>::type
  {
    typedef ReturnTag returns;
    typedef TraversalTag traversal;
  };


} // namespace boost

#endif // BOOST_ITERATOR_CATEGORIES_HPP
