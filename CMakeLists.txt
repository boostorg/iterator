# Copyright 2018 Peter Dimov
# Copyright 2018 Andrey Semashev
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.20)

project(boost_iterator VERSION "${BOOST_SUPERPROJECT_VERSION}" LANGUAGES CXX)

add_library(boost_iterator INTERFACE)
add_library(Boost::iterator ALIAS boost_iterator)

target_include_directories(boost_iterator INTERFACE include)

target_link_libraries(boost_iterator
    INTERFACE
        Boost::assert
        Boost::concept_check
        Boost::config
        Boost::core
        Boost::detail
        Boost::fusion
        Boost::mpl
        Boost::optional
        Boost::type_traits
        Boost::utility
)
