// Copyright (C) 2018 Andrey Semashev
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// The following Boost.MPL includes are needed to mitigate the missing includes in Boost.Range.
// They can be removed once https://github.com/boostorg/range/pull/154 is merged.
// -- Begin workaround includes
#include <boost/mpl/assert.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/identity.hpp>
// -- End of workaround includes

#include <boost/range/distance.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/iterator/distance.hpp>

int main()
{
    // Test that boost::distance from Boost.Range works with boost::distance from Boost.Iterator
    // (https://github.com/boostorg/iterator/commit/b844c8df530c474ec1856870b9b0de5f487b84d4#commitcomment-30603668)

    typedef boost::iterator_range<const char*> range_type;
    range_type range;

    (void)boost::distance(range);

    return 0;
}
