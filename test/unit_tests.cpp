// Copyright David Abrahams 2003. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
#include <boost/iterator/iterator_adaptor.hpp>

struct X { int a; };

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
namespace boost { namespace detail {
template<> struct iterator_traits<X*>
  : ptr_iter_traits<X> {};
}}
#endif 

struct Xiter : boost::iterator_adaptor<Xiter,X*>
{
    Xiter();
    Xiter(X* p) : boost::iterator_adaptor<Xiter, X*>(p) {}
};

void take_xptr(X*) {}
void operator_arrow_test()
{
    // check that the operator-> result is a pointer for lvalue iterators
    X x;
    take_xptr(Xiter(&x).operator->());
}

int main()
{
    operator_arrow_test();
    return 0;
}

