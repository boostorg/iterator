# Copyright 2019 Mike Dev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt
#
# NOTE: CMake support for Boost.Iterator is currently experimental at best
#       and the interface is likely to change in the future

###### Collect all tests files ######
file(GLOB test_files *.cpp)

###### Remove test that can not be run (yet) from CMake for various reasons  ############
# No support for compile fail tests
list( FILTER test_files EXCLUDE REGEX "_fail.cpp" )
list( FILTER test_files EXCLUDE REGEX "iter_archetype_default_ctor.cpp" )

# No support for compilation only (without linking a complete program)
list( FILTER test_files EXCLUDE REGEX "constant_iter_arrow.cpp" )

# Requires Boost.Range which doesn't support cmake yet
list( FILTER test_files EXCLUDE REGEX "range_distance_compat_test.cpp" )

# Requires Boost.Test which doesn't support cmake yet
list( FILTER test_files EXCLUDE REGEX "permutation_iterator_test.cpp" )

# Requires Boost.Assign which doesn't support cmake yet
list( FILTER test_files EXCLUDE REGEX "zip_iterator_test_std_pair.cpp" )
list( FILTER test_files EXCLUDE REGEX "zip_iterator_test_std_tuple.cpp" )
list( FILTER test_files EXCLUDE REGEX "zip_iterator_test_fusion.cpp" )

# Broken
# https://svn.boost.org/trac/boost/ticket/11572
list( FILTER test_files EXCLUDE REGEX "zip_iterator_test2_fusion_deque.cpp" )


###### Create tests ######
foreach( file IN LISTS test_files)

    get_filename_component( core_name ${file} NAME_WE )
    set( test_name test_boost_iterator_${core_name} )

    add_executable( ${test_name} ${file} )
    target_link_libraries( ${test_name}
        PRIVATE
            Boost::iterator

            # Add additional dependencies that are required from some tests
            # TODO: Add only dependencies that are required by individual test
            Boost::array
            Boost::bind
            Boost::container
            Boost::core
    )
    add_test( NAME ${test_name} COMMAND ${test_name} )

endforeach()
