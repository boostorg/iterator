// (C) Copyright Jeremy Siek 2002. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#include <boost/iterator/iterator_concepts.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp> // remove

struct new_iterator
  : public boost::iterator< boost::iterator_tag<
    boost::mutable_lvalue_iterator_tag
    , boost::random_access_traversal_tag>, int>
{
  int& operator*() const { return *m_x; }
  new_iterator& operator++() { return *this; }
  new_iterator operator++(int) { return *this; }
  new_iterator& operator--() { return *this; }
  new_iterator operator--(int) { return *this; }
  new_iterator& operator+=(std::ptrdiff_t) { return *this; }
  new_iterator operator+(std::ptrdiff_t) { return *this; }
  new_iterator& operator-=(std::ptrdiff_t) { return *this; }
  std::ptrdiff_t operator-(const new_iterator&) const { return 0; }
  new_iterator operator-(std::ptrdiff_t) const { return *this; }
  bool operator==(const new_iterator&) const { return false; }
  bool operator!=(const new_iterator&) const { return false; }
  bool operator<(const new_iterator&) const { return false; }
  int* m_x;
};
new_iterator operator+(std::ptrdiff_t, new_iterator x) { return x; }

struct old_iterator
  : public boost::iterator<std::random_access_iterator_tag, int>
{
  int& operator*() const { return *m_x; }
  old_iterator& operator++() { return *this; }
  old_iterator operator++(int) { return *this; }
  old_iterator& operator--() { return *this; }
  old_iterator operator--(int) { return *this; }
  old_iterator& operator+=(std::ptrdiff_t) { return *this; }
  old_iterator operator+(std::ptrdiff_t) { return *this; }
  old_iterator& operator-=(std::ptrdiff_t) { return *this; }
  old_iterator operator-(std::ptrdiff_t) const { return *this; }
  std::ptrdiff_t operator-(const old_iterator&) const { return 0; }
  bool operator==(const old_iterator&) const { return false; }
  bool operator!=(const old_iterator&) const { return false; }
  bool operator<(const old_iterator&) const { return false; }
  int* m_x;
};
old_iterator operator+(std::ptrdiff_t, old_iterator x) { return x; }

int
main()
{
  BOOST_STATIC_ASSERT( boost::detail::is_random_access_traversal_iterator<boost::random_access_traversal_tag>::value );

  typedef boost::iterator_tag< boost::mutable_lvalue_iterator_tag, boost::random_access_traversal_tag > tag;

  // BOOST_STATIC_ASSERT((boost::detail::is_random_access_iterator<tag>::value));
  BOOST_STATIC_ASSERT((boost::is_same<tag::returns, boost::mutable_lvalue_iterator_tag>::value));
  BOOST_STATIC_ASSERT((boost::is_same<tag::traversal, boost::random_access_traversal_tag>::value));

  // BOOST_STATIC_ASSERT((boost::detail::is_random_access_iterator<new_iterator::iterator_category>::value));
  BOOST_STATIC_ASSERT((boost::is_same<new_iterator::iterator_category::returns, boost::mutable_lvalue_iterator_tag>::value));
  BOOST_STATIC_ASSERT((boost::is_same<new_iterator::iterator_category::traversal, boost::random_access_traversal_tag>::value));

  typedef boost::traversal_category<new_iterator>::type traversal_category;

  //  BOOST_STATIC_ASSERT(boost::detail::has_traversal<new_iterator::iterator_category>::value);
  BOOST_STATIC_ASSERT(boost::detail::is_new_iterator_tag<new_iterator::iterator_category>::value);


  BOOST_STATIC_ASSERT((boost::is_same<traversal_category, boost::random_access_traversal_tag>::value));


#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
  boost::function_requires<
    boost_concepts::MutableLvalueIteratorConcept<int*> >();
  boost::function_requires<
    boost_concepts::RandomAccessTraversalConcept<int*> >();

  boost::function_requires<
    boost_concepts::ConstantLvalueIteratorConcept<const int*> >();
  boost::function_requires<
    boost_concepts::RandomAccessTraversalConcept<const int*> >();
#endif

  boost::function_requires<
    boost_concepts::MutableLvalueIteratorConcept<new_iterator> >();
  boost::function_requires<
    boost_concepts::RandomAccessTraversalConcept<new_iterator> >();

  boost::function_requires<
    boost_concepts::MutableLvalueIteratorConcept<old_iterator> >();
  boost::function_requires<
    boost_concepts::RandomAccessTraversalConcept<old_iterator> >();
  return 0;
}
