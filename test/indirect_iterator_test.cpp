//  (C) Copyright Jeremy Siek 1999. Permission to copy, use, modify,
//  sell and distribute this software is granted provided this
//  copyright notice appears in all copies. This software is provided
//  "as is" without express or implied warranty, and with no claim as
//  to its suitability for any purpose.

//  Revision History
//  22 Nov 2002 Thomas Witt
//       Added interoperability check.
//  08 Mar 2001   Jeremy Siek
//       Moved test of indirect iterator into its own file. It to
//       to be in iterator_adaptor_test.cpp.

#include <boost/config.hpp>
#include <iostream>
#include <algorithm>

#include <boost/iterator/iterator_adaptors.hpp>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/iterator/new_iterator_tests.hpp>

#include <boost/detail/workaround.hpp>

#include <boost/concept_archetype.hpp>
#include <boost/concept_check.hpp>
#include <boost/shared_ptr.hpp>

#include <stdlib.h>
#include <deque>
#include <set>

struct my_iterator_tag : public std::random_access_iterator_tag { };

using boost::dummyT;

typedef std::deque<int> storage;
typedef std::deque<int*> pointer_deque;
typedef std::set<storage::iterator> iterator_set;

template <class Container>
struct indirect_iterator_pair_generator
{
    typedef boost::indirect_iterator<typename Container::iterator> iterator;
    typedef boost::indirect_iterator<typename Container::const_iterator> const_iterator;
};

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
namespace boost { namespace detail
{
  template<> struct iterator_traits<int*>
  : ptr_iter_traits<int> {};
  
  template<> struct iterator_traits<dummyT*>
  : ptr_iter_traits<dummyT> {};
  
  template<> struct iterator_traits<dummyT const*>
  : ptr_iter_traits<dummyT, dummyT const> {};
  
  template<> struct iterator_traits<dummyT**>
  : ptr_iter_traits<dummyT*> {};
  
  template<> struct iterator_traits<dummyT const*const*>
  : ptr_iter_traits<dummyT const*, dummyT const*const> {};
}}
#endif

void more_indirect_iterator_tests()
{
// For some reason all heck breaks loose in the compiler under these conditions.
    storage store(1000);
    std::generate(store.begin(), store.end(), rand);
    
    pointer_deque ptr_deque;
    iterator_set iter_set;

    for (storage::iterator p = store.begin(); p != store.end(); ++p)
    {
        ptr_deque.push_back(&*p);
        iter_set.insert(p);
    }

    typedef indirect_iterator_pair_generator<pointer_deque> IndirectDeque;

    IndirectDeque::iterator db(ptr_deque.begin());
    IndirectDeque::iterator de(ptr_deque.end());
    assert(static_cast<std::size_t>(de - db) == store.size());
    assert(db + store.size() == de);
    IndirectDeque::const_iterator dci(db);
    
    assert(dci == db);
    
    // Older Dinkumware and GCC standard lib don't supply symmetric constant/mutable
    // iterator operators
#if !defined(BOOST_MSVC_STD_ITERATOR) \
    && !BOOST_WORKAROUND(_CPPLIB_VER, <= 310) \
    && !BOOST_WORKAROUND(__GNUC__, <= 2 && !defined(__SGI_STL_PORT))
    
    assert(db == dci);
    
    assert(dci != de);
    assert(dci < de);
    assert(dci <= de);
#endif
    
    assert(de >= dci);
    assert(de > dci);
    
    dci = de;
    assert(dci == de);

    boost::random_access_iterator_test(db + 1, store.size() - 1, boost::next(store.begin()));
    
    *db = 999;
    assert(store.front() == 999);

    // Borland C++ is getting very confused about the typedefs here
    typedef boost::indirect_iterator<iterator_set::iterator> indirect_set_iterator;
    typedef boost::indirect_iterator<iterator_set::const_iterator> const_indirect_set_iterator;

    indirect_set_iterator sb(iter_set.begin());
    indirect_set_iterator se(iter_set.end());
    const_indirect_set_iterator sci(iter_set.begin());
    assert(sci == sb);
    assert(sci != se);
    sci = se;
    assert(sci == se);
    
    *boost::prior(se) = 888;
    assert(store.back() == 888);
    assert(std::equal(sb, se, store.begin()));

    boost::bidirectional_iterator_test(boost::next(sb), store[1], store[2]);
    assert(std::equal(db, de, store.begin()));
}

int
main()
{
  dummyT array[] = { dummyT(0), dummyT(1), dummyT(2), 
                     dummyT(3), dummyT(4), dummyT(5) };
  const int N = sizeof(array)/sizeof(dummyT);

  typedef std::deque<boost::shared_ptr<dummyT> > shared_t;
  shared_t shared;
  
  // Concept checks
  {
    typedef boost::indirect_iterator<int**>       iter_t;
    typedef boost::indirect_iterator<int* const*> c_iter_t;

    // Older Dinkumware and GCC standard lib don't supply symmetric constant/mutable
    // iterator operators
#if !defined(BOOST_MSVC_STD_ITERATOR) && (!defined(_CPPLIB_VER) || _CPPLIB_VER > 310)        \
    && (__GNUC__ != 2 || defined(__SGI_STL_PORT))
    boost::function_requires< boost_concepts::InteroperableConcept<iter_t, c_iter_t> >();
#endif
  }

  // Test indirect_iterator_generator
  {
      for (int jj = 0; jj < N; ++jj)
          shared.push_back(boost::shared_ptr<dummyT>(new dummyT(jj)));
      
      dummyT* ptr[N];
      for (int k = 0; k < N; ++k)
          ptr[k] = array + k;

      typedef boost::indirect_iterator<dummyT**> indirect_iterator;

      typedef boost::indirect_iterator<dummyT const* const*> const_indirect_iterator;

      indirect_iterator i(ptr);
      boost::random_access_iterator_test(i, N, array);

#if __GNUC__ != 2
      // We're having trouble auto-detecting smart pointers with
      // gcc-2.95 via the nested element_type member. However, we
      // really ought to have a specializable is_pointer template
      // which can be used instead with something like
      // boost/python/pointee.hpp to find the value_type.
      
      boost::random_access_iterator_test(
          boost::indirect_iterator<shared_t::iterator>(shared.begin())
          , N, array);
#endif 

      boost::random_access_iterator_test(boost::make_indirect_iterator(ptr), N, array);
    
      // check operator->
      assert((*i).m_x == i->foo());

      const_indirect_iterator j(ptr);
      boost::random_access_iterator_test(j, N, array);

    
      dummyT const*const* const_ptr = ptr;
      boost::random_access_iterator_test(boost::make_indirect_iterator(const_ptr), N, array);
      
      boost::const_nonconst_iterator_test(i, ++j);

      more_indirect_iterator_tests();
  }
  std::cout << "test successful " << std::endl;
  return 0;
}
