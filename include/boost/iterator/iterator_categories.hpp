// (C) Copyright Jeremy Siek 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_ITERATOR_CATEGORIES_HPP
# define BOOST_ITERATOR_CATEGORIES_HPP

# include <boost/config.hpp>
# include <boost/detail/iterator.hpp>
# include <boost/iterator/detail/config_def.hpp>

# include <boost/detail/workaround.hpp>

# include <boost/mpl/if.hpp>
# include <boost/mpl/and.hpp>
# include <boost/mpl/apply_if.hpp>
# include <boost/mpl/identity.hpp>
# include <boost/mpl/placeholders.hpp>
# include <boost/mpl/aux_/lambda_support.hpp>

# include <boost/type_traits/is_reference.hpp>
# include <boost/type_traits/is_convertible.hpp>
# include <boost/type_traits/is_const.hpp>

namespace boost {

//
// Traversal Categories
//
struct incrementable_traversal_tag {};
  
struct single_pass_traversal_tag
  : incrementable_traversal_tag {};
  
struct forward_traversal_tag
  : single_pass_traversal_tag {};
  
struct bidirectional_traversal_tag
  : forward_traversal_tag {};
  
struct random_access_traversal_tag
  : bidirectional_traversal_tag {};

namespace detail
{
  struct input_output_iterator_tag
    : std::input_iterator_tag, std::output_iterator_tag {};

  template <class Value, class Reference, class Traversal>
  struct old_iterator_category
  {
      typedef typename mpl::
# if BOOST_WORKAROUND(BOOST_MSVC, == 1200)
      if_
# else
      apply_if
# endif 
      <
          mpl::and_<
              is_reference<Reference>
            , is_convertible<Traversal,forward_traversal_tag>
          >
        , mpl::apply_if<
              is_convertible<Traversal,random_access_traversal_tag>
            , mpl::identity<std::random_access_iterator_tag>
            , mpl::if_<
                  is_convertible<Traversal,bidirectional_traversal_tag>
                , std::bidirectional_iterator_tag
                , std::forward_iterator_tag
              >
          >
# if BOOST_WORKAROUND(BOOST_MSVC, == 1200)
           ::type
# endif 
        , typename mpl::apply_if<
              mpl::and_<
                  is_convertible<Traversal, single_pass_traversal_tag>
                , is_convertible<Reference, Value>
              >
            , mpl::if_<
                  is_const<Value>
                , std::input_iterator_tag
                , input_output_iterator_tag
              >
            , mpl::identity<std::output_iterator_tag>
          >
# if BOOST_WORKAROUND(BOOST_MSVC, == 1200)
           ::type
# endif 
      >::type type;
  };

  //
  // Convert a "strictly old-style" iterator category to a traversal
  // tag.  This is broken out into a separate metafunction to reduce
  // the cost of instantiating iterator_category_to_traversal, below,
  // for new-style types.
  //
  template <class Cat>
  struct old_style_category_to_traversal
    : mpl::apply_if<
          is_convertible<Cat,std::random_access_iterator_tag>
        , mpl::identity<random_access_traversal_tag>
        , mpl::apply_if<
              is_convertible<Cat,std::bidirectional_iterator_tag>
            , mpl::identity<bidirectional_traversal_tag>
            , mpl::apply_if<
                  is_convertible<Cat,std::forward_iterator_tag>
                , mpl::identity<forward_traversal_tag>
                , mpl::apply_if<
                      is_convertible<Cat,std::input_iterator_tag>
                    , mpl::identity<single_pass_traversal_tag>
                    , mpl::apply_if<
                          is_convertible<Cat,std::output_iterator_tag>
                        , mpl::identity<incrementable_traversal_tag>
                        , void
                      >
                  >
              >
          >
      >
  {};
  
} // namespace detail


//
// Convert an iterator category into a traversal tag
//
template <class Cat>
struct iterator_category_to_traversal
  : mpl::apply_if< // if already convertible to a traversal tag, we're done.
        is_convertible<Cat,incrementable_traversal_tag>
      , mpl::identity<Cat>
      , detail::old_style_category_to_traversal<Cat>
    >
{};

//
// To be used for a new-style iterator's iterator_category; provides
// implicit conversion to the appropriate old-style iterator category.
//
// If Value is const the result will not be convertible to
// output_iterator_tag.
//
// Otherwise, if Traversal == single_pass_traversal_tag, the following
// conditions will result in a tag that is convertible both to
// input_iterator_tag and output_iterator_tag:
//
//    1. Reference is a reference to non-const
//    2. Reference is not a reference and is convertible to Value
//
template <class Value, class Reference, class Traversal>
struct iterator_tag
  : detail::old_iterator_category<Value, Reference, Traversal>::type
{
    operator Traversal() { return Traversal(); }
# if 0
    typedef typename detail::old_iterator_category<
        Value, Reference, Traversal
    >::type old_category;
    
    operator old_category() const { return old_category(); }
# endif 
};

// Trait to get an iterator's traversal category
template <class Iterator = mpl::_1>
struct iterator_traversal
  : iterator_category_to_traversal<
        typename boost::detail::iterator_traits<Iterator>::iterator_category
    >
{};

# ifdef BOOST_MPL_NO_FULL_LAMBDA_SUPPORT
// Hack because BOOST_MPL_AUX_LAMBDA_SUPPORT doesn't seem to work
// out well.  Instantiating the nested apply template also
// requires instantiating iterator_traits on the
// placeholder. Instead we just specialize it as a metafunction
// class.
template <>
struct iterator_traversal<mpl::_1>
{
    template <class T>
    struct apply : iterator_traversal<T>
    {};
};
template <>
struct iterator_traversal<mpl::_>
  : iterator_traversal<mpl::_1>
{};
# endif


} // namespace boost

#include <boost/iterator/detail/config_undef.hpp>

#endif // BOOST_ITERATOR_CATEGORIES_HPP
