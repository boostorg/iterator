#include <boost/iterator/iterator_adaptors.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/concept_check.hpp>
#include <boost/cstdlib.hpp>
#include <list>

int main()
{
  {
    typedef boost::reverse_iterator<int*, int, int&, int*, 
      boost::iterator_tag<boost::mutable_lvalue_iterator_tag, boost::random_access_traversal_tag>, 
      std::ptrdiff_t> rev_iter;
    boost::function_requires< boost_concepts::MutableLvalueIteratorConcept<rev_iter> >();
    boost::function_requires< boost_concepts::RandomAccessTraversalConcept<rev_iter> >();
    boost::function_requires< boost::RandomAccessIteratorConcept<rev_iter> >();
  }
  {
    typedef boost::reverse_iterator<std::list<int>::const_iterator, int, const int&, const int*, 
      boost::iterator_tag<boost::constant_lvalue_iterator_tag, boost::bidirectional_traversal_tag>
      , std::ptrdiff_t> rev_iter;
    boost::function_requires< boost_concepts::ConstantLvalueIteratorConcept<rev_iter> >();
    boost::function_requires< boost_concepts::BidirectionalTraversalConcept<rev_iter> >();
    boost::function_requires< boost::BidirectionalIteratorConcept<rev_iter> >();
  }

  return boost::exit_success;
}
