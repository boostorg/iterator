#include <boost/iterator/iterator_adaptors.hpp>
#include <boost/cstdlib.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>

int main()
{
  int x[] = { 1, 2, 3, 4 };
  boost::reverse_iterator<int*, int, int&, int*, std::random_access_iterator_tag, std::ptrdiff_t>
    first(x + 4), last(x);
  std::copy(first, last, std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
  return 0;
}
