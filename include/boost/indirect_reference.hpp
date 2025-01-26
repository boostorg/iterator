#ifndef INDIRECT_REFERENCE_DWA200415_HPP
#define INDIRECT_REFERENCE_DWA200415_HPP

//
// Copyright David Abrahams 2004. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// typename indirect_reference<P>::type provides the type of *p.
//
// http://www.boost.org/libs/iterator/doc/pointee.html
//

#include <boost/detail/is_incrementable.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/pointee.hpp>

#include <type_traits>

namespace boost {
namespace detail {

template< typename P >
struct smart_ptr_reference
{
    using type = typename boost::pointee<P>::type&;
};

} // namespace detail

template< typename P >
struct indirect_reference :
    std::conditional<
        detail::is_incrementable<P>::value,
        iterator_reference<P>,
        detail::smart_ptr_reference<P>
    >::type
{
};

} // namespace boost

#endif // INDIRECT_REFERENCE_DWA200415_HPP
