// Copyright (C) 2017 Michel Morin.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ITERATOR_ADVANCE_HPP
#define BOOST_ITERATOR_ADVANCE_HPP

#include <boost/config.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_traits.hpp>

namespace boost {
namespace iterators {

    namespace detail {
        template <typename InputIterator>
        inline BOOST_CXX14_CONSTEXPR void
        advance_impl(
            InputIterator& it
          , typename iterator_difference<InputIterator>::type n
          , incrementable_traversal_tag
        )
        {
            while (n > 0) {
                ++it;
                --n;
            }
        }

        template <typename BidirectionalIterator>
        inline BOOST_CXX14_CONSTEXPR void
        advance_impl(
            BidirectionalIterator& it
          , typename iterator_difference<BidirectionalIterator>::type n
          , bidirectional_traversal_tag
        )
        {
            if (n >= 0) {
                while (n > 0) {
                    ++it;
                    --n;
                }
            }
            else {
                while (n < 0) {
                    --it;
                    ++n;
                }
            }
        }

        template <typename RandomAccessIterator>
        inline BOOST_CXX14_CONSTEXPR void
        advance_impl(
            RandomAccessIterator& it
          , typename iterator_difference<RandomAccessIterator>::type n
          , random_access_traversal_tag
        )
        {
            it += n;
        }
    }

    namespace advance_adl_barrier {
        template <typename InputIterator>
        inline BOOST_CXX14_CONSTEXPR void
        advance(InputIterator& it, typename iterator_difference<InputIterator>::type n)
        {
            detail::advance_impl(
                it, n, typename iterator_traversal<InputIterator>::type()
            );
        }
    }

    using namespace advance_adl_barrier;

} // namespace iterators

using iterators::advance;

} // namespace boost

#endif
