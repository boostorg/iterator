#include <boost/iterator/reverse_iterator.hpp>
#include <boost/static_assert.hpp>
#include <boost/cstdlib.hpp>

int main()
{
  {
    typedef boost::reverse_iterator<int*>  rev_iter1;
    typedef boost::reverse_iterator<char*> rev_iter2;

    BOOST_STATIC_ASSERT((boost::is_convertible<rev_iter1, rev_iter2>::value));
  }

  return boost::exit_success;
}
