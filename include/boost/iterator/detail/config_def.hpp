// (C) Copyright David Abrahams 2002.
// (C) Copyright Jeremy Siek    2002.
// (C) Copyright Thomas Witt    2002.
// Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

// no include guard multiple inclusion intended

//
// This is a temporary workaround until the bulk of this is
// available in boost config.
// 23/02/03 thw
//

#include <boost/config.hpp> // for prior
#include <boost/detail/workaround.hpp>

#if BOOST_WORKAROUND(BOOST_MSVC,  <= 1300)                      \
 || BOOST_WORKAROUND(__GNUC__, <= 2 && __GNUC_MINOR__ <= 95)    \
 || BOOST_WORKAROUND(__MWERKS__, <= 0x3000)
#  define BOOST_NO_SFINAE // "Substitution Failure Is Not An Error not implemented"
#endif

#if BOOST_WORKAROUND(BOOST_MSVC, <=1200)
#  define BOOST_ARG_DEP_TYPENAME
#else
#  define BOOST_ARG_DEP_TYPENAME typename
#endif

#if BOOST_WORKAROUND(__MWERKS__, <=0x2407)
#  define BOOST_NO_IS_CONVERTIBLE // "is_convertible doesn't always work"
#endif

#if BOOST_WORKAROUND(__GNUC__, == 2 && __GNUC_MINOR__ == 95)    \
  || BOOST_WORKAROUND(__MWERKS__, <= 0x2407)                    \
  || BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x551))
# define BOOST_NO_MPL_AUX_HAS_XXX  // "MPL's has_xxx facility doesn't work"
#endif 

// no include guard multiple inclusion intended
